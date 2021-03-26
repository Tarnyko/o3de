/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include <AzCore/std/smart_ptr/weak_ptr.h>
#include <AzCore/Jobs/JobFunction.h>

#include <Authentication/AWSCognitoAuthenticationProvider.h>
#include <Authentication/AuthenticationProviderTypes.h>
#include <Authentication/AuthenticationProviderBus.h>
#include <AWSClientAuthBus.h>
#include <AWSCoreBus.h>

#include <aws/cognito-idp/model/InitiateAuthRequest.h>
#include <aws/cognito-idp/model/InitiateAuthResult.h>
#include <aws/cognito-idp/model/RespondToAuthChallengeRequest.h>
#include <aws/cognito-idp/model/RespondToAuthChallengeResult.h>
#include <aws/cognito-idp/CognitoIdentityProviderClient.h>
#include <aws/cognito-idp/CognitoIdentityProviderErrors.h>

namespace AWSClientAuth
{

    constexpr char COGNITO_IDP_SETTINGS_PATH[] = "/AWS/CognitoIDP";
    constexpr char COGNITO_USERNAME_KEY[] = "USERNAME";
    constexpr char COGNITO_PASSWORD_KEY[] = "PASSWORD";
    constexpr char COGNITO_REFRESH_TOKEN_AUTHPARAM_KEY[] = "REFRESH_TOKEN";
    constexpr char COGNITO_SMS_MFA_CODE_KEY[] = "SMS_MFA_CODE";

    AWSCognitoAuthenticationProvider::AWSCognitoAuthenticationProvider()
    {
        m_settings = AZStd::make_unique<AWSCognitoProviderSetting>();
    }

    AWSCognitoAuthenticationProvider::~AWSCognitoAuthenticationProvider()
    {
        m_settings.reset();
    }

    bool AWSCognitoAuthenticationProvider::Initialize(AZStd::weak_ptr<AZ::SettingsRegistryInterface> settingsRegistry)
    {
        if (!settingsRegistry.lock()->GetObject(m_settings.get(), azrtti_typeid(m_settings.get()), COGNITO_IDP_SETTINGS_PATH))
        {
            AZ_Warning("AWSCognitoAuthenticationProvider", true, "Failed to get settings object for path %s", COGNITO_IDP_SETTINGS_PATH);
            return false;
        }
        return true;
    }


    void AWSCognitoAuthenticationProvider::PasswordGrantSingleFactorSignInAsync(const AZStd::string& username, const AZStd::string& password)
    {
        InitiateAuthInternalAsync(username, password, [this](Aws::CognitoIdentityProvider::Model::InitiateAuthOutcome initiateAuthOutcome)
        {
            if (initiateAuthOutcome.IsSuccess())
            {
                Aws::CognitoIdentityProvider::Model::InitiateAuthResult initiateAuthResult{ initiateAuthOutcome.GetResult() };
                if (initiateAuthResult.GetChallengeName() == Aws::CognitoIdentityProvider::Model::ChallengeNameType::NOT_SET)
                {
                    Aws::CognitoIdentityProvider::Model::AuthenticationResultType authenticationResult = initiateAuthResult.GetAuthenticationResult();
                    UpdateTokens(authenticationResult);
                    AuthenticationProviderNotificationBus::Broadcast(&AuthenticationProviderNotifications::OnPasswordGrantSingleFactorSignInSuccess
                        , m_authenticationTokens);
                }
                else
                {
                    AuthenticationProviderNotificationBus::Broadcast(&AuthenticationProviderNotifications::OnPasswordGrantSingleFactorSignInFail
                        , AZStd::string::format("Unexpected Challenge type: %s"
                            , Aws::CognitoIdentityProvider::Model::ChallengeNameTypeMapper::GetNameForChallengeNameType(initiateAuthResult.GetChallengeName()).c_str()));
                }
            }
            else
            {
                Aws::Client::AWSError<Aws::CognitoIdentityProvider::CognitoIdentityProviderErrors> error = initiateAuthOutcome.GetError();
                AuthenticationProviderNotificationBus::Broadcast(&AuthenticationProviderNotifications::OnPasswordGrantSingleFactorSignInFail, error.GetMessage().c_str());
            }
        });
    }

    void AWSCognitoAuthenticationProvider::PasswordGrantMultiFactorSignInAsync(const AZStd::string& username, const AZStd::string& password)
    {
        InitiateAuthInternalAsync(username, password, [this](Aws::CognitoIdentityProvider::Model::InitiateAuthOutcome initiateAuthOutcome)
        {
            if (initiateAuthOutcome.IsSuccess())
            {
                Aws::CognitoIdentityProvider::Model::InitiateAuthResult initiateAuthResult{ initiateAuthOutcome.GetResult() };
                if (initiateAuthResult.GetChallengeName() == Aws::CognitoIdentityProvider::Model::ChallengeNameType::SMS_MFA)
                {
                    Aws::CognitoIdentityProvider::Model::AuthenticationResultType authenticationResult = initiateAuthResult.GetAuthenticationResult();
                    // Call on sign in success for MFA
                    AuthenticationProviderNotificationBus::Broadcast(&AuthenticationProviderNotifications::OnPasswordGrantMultiFactorSignInSuccess);
                    m_session = initiateAuthResult.GetSession().c_str();
                }
                else
                {
                    AuthenticationProviderNotificationBus::Broadcast(&AuthenticationProviderNotifications::OnPasswordGrantMultiFactorSignInFail
                        , AZStd::string::format("Unexpected Challenge type: %s"
                            , Aws::CognitoIdentityProvider::Model::ChallengeNameTypeMapper::GetNameForChallengeNameType(initiateAuthResult.GetChallengeName()).c_str()));
                }
            }
            else
            {
                Aws::Client::AWSError<Aws::CognitoIdentityProvider::CognitoIdentityProviderErrors> error = initiateAuthOutcome.GetError();
                AuthenticationProviderNotificationBus::Broadcast(&AuthenticationProviderNotifications::OnPasswordGrantMultiFactorSignInFail, error.GetMessage().c_str());
            }
        });
    }

    // Call RespondToAuthChallenge for Cognito authentication flow.
    // Refer https://docs.aws.amazon.com/cognito/latest/developerguide/amazon-cognito-user-pools-authentication-flow.html.
    void AWSCognitoAuthenticationProvider::PasswordGrantMultiFactorConfirmSignInAsync(const AZStd::string& username, const AZStd::string& confirmationCode)
    {
        std::shared_ptr<Aws::CognitoIdentityProvider::CognitoIdentityProviderClient> cognitoIdentityProviderClient =
            AZ::Interface<IAWSClientAuthRequests>::Get()->GetCognitoIDPClient();

        AZ::JobContext* jobContext = nullptr;
        AWSCore::AWSCoreRequestBus::BroadcastResult(jobContext, &AWSCore::AWSCoreRequests::GetDefaultJobContext);
        AZ::Job* confirmSignInJob = AZ::CreateJobFunction([this, cognitoIdentityProviderClient, confirmationCode, username]()
        {
            // Set Request parameters for SMS Multi factor authentication.
            // Note: Email MFA is no longer supported by Cognito, use SMS as MFA
            Aws::CognitoIdentityProvider::Model::RespondToAuthChallengeRequest respondToAuthChallengeRequest;
            respondToAuthChallengeRequest.SetClientId(m_settings->m_appClientId.c_str());
            respondToAuthChallengeRequest.AddChallengeResponses(COGNITO_SMS_MFA_CODE_KEY, confirmationCode.c_str());
            respondToAuthChallengeRequest.AddChallengeResponses(COGNITO_USERNAME_KEY, username.c_str());
            respondToAuthChallengeRequest.SetChallengeName(Aws::CognitoIdentityProvider::Model::ChallengeNameType::SMS_MFA);
            respondToAuthChallengeRequest.SetSession(m_session.c_str());

            Aws::CognitoIdentityProvider::Model::RespondToAuthChallengeOutcome respondToAuthChallengeOutcome{ cognitoIdentityProviderClient->RespondToAuthChallenge(respondToAuthChallengeRequest) };
            if (respondToAuthChallengeOutcome.IsSuccess())
            {
                Aws::CognitoIdentityProvider::Model::RespondToAuthChallengeResult respondToAuthChallengeResult{ respondToAuthChallengeOutcome.GetResult() };
                if (respondToAuthChallengeResult.GetChallengeName() == Aws::CognitoIdentityProvider::Model::ChallengeNameType::NOT_SET)
                {
                    Aws::CognitoIdentityProvider::Model::AuthenticationResultType authenticationResult = respondToAuthChallengeResult.GetAuthenticationResult();
                    UpdateTokens(authenticationResult);
                    AuthenticationProviderNotificationBus::Broadcast(
                        &AuthenticationProviderNotifications::OnPasswordGrantMultiFactorConfirmSignInSuccess, m_authenticationTokens);
                }
            }
            else
            {
                Aws::Client::AWSError<Aws::CognitoIdentityProvider::CognitoIdentityProviderErrors> error = respondToAuthChallengeOutcome.GetError();
                AuthenticationProviderNotificationBus::Broadcast(&AuthenticationProviderNotifications::OnPasswordGrantMultiFactorConfirmSignInFail, error.GetMessage().c_str());
            }
        }, true, jobContext);
        confirmSignInJob->Start();
    }

    void AWSCognitoAuthenticationProvider::DeviceCodeGrantSignInAsync()
    {
        AZ_Assert(true, "Not supported");
    }

    void AWSCognitoAuthenticationProvider::DeviceCodeGrantConfirmSignInAsync()
    {
        AZ_Assert(true, "Not supported");
    }

    void AWSCognitoAuthenticationProvider::RefreshTokensAsync()
    {
        std::shared_ptr<Aws::CognitoIdentityProvider::CognitoIdentityProviderClient> cognitoIdentityProviderClient =
            AZ::Interface<IAWSClientAuthRequests>::Get()->GetCognitoIDPClient();

        AZ::JobContext* jobContext = nullptr;
        AWSCore::AWSCoreRequestBus::BroadcastResult(jobContext, &AWSCore::AWSCoreRequests::GetDefaultJobContext);

        AZ::Job* initiateAuthJob = AZ::CreateJobFunction([this, cognitoIdentityProviderClient]()
        {
            // Set Request parameters.
            Aws::CognitoIdentityProvider::Model::InitiateAuthRequest initiateAuthRequest;
            initiateAuthRequest.SetClientId(m_settings->m_appClientId.c_str());
            initiateAuthRequest.SetAuthFlow(Aws::CognitoIdentityProvider::Model::AuthFlowType::REFRESH_TOKEN_AUTH);

            // Set username and password for Password grant/ Initiate Auth flow.
            Aws::Map<Aws::String, Aws::String> authParameters
            {
                {COGNITO_REFRESH_TOKEN_AUTHPARAM_KEY, GetAuthenticationTokens().GetRefreshToken().c_str()}
            };
            initiateAuthRequest.SetAuthParameters(authParameters);

            Aws::CognitoIdentityProvider::Model::InitiateAuthOutcome initiateAuthOutcome{ cognitoIdentityProviderClient->InitiateAuth(initiateAuthRequest) };
            if (initiateAuthOutcome.IsSuccess())
            {
                Aws::CognitoIdentityProvider::Model::InitiateAuthResult initiateAuthResult{ initiateAuthOutcome.GetResult() };
                if (initiateAuthResult.GetChallengeName() == Aws::CognitoIdentityProvider::Model::ChallengeNameType::NOT_SET)
                {
                    Aws::CognitoIdentityProvider::Model::AuthenticationResultType authenticationResult = initiateAuthResult.GetAuthenticationResult();
                    UpdateTokens(authenticationResult);
                    AuthenticationProviderNotificationBus::Broadcast(&AuthenticationProviderNotifications::OnRefreshTokensSuccess, m_authenticationTokens);
                }
                else
                {
                    AuthenticationProviderNotificationBus::Broadcast(&AuthenticationProviderNotifications::OnRefreshTokensFail
                        , AZStd::string::format("Unexpected Challenge type: %s"
                            , Aws::CognitoIdentityProvider::Model::ChallengeNameTypeMapper::GetNameForChallengeNameType(initiateAuthResult.GetChallengeName()).c_str()));
                }
            }
            else
            {
                Aws::Client::AWSError<Aws::CognitoIdentityProvider::CognitoIdentityProviderErrors> error = initiateAuthOutcome.GetError();
                AuthenticationProviderNotificationBus::Broadcast(&AuthenticationProviderNotifications::OnRefreshTokensFail, error.GetMessage().c_str());
            }
        }, true, jobContext);
        initiateAuthJob->Start();
    }

    // Call InitiateAuth for Cognito authentication flow.
    // Refer https://docs.aws.amazon.com/cognito/latest/developerguide/amazon-cognito-user-pools-authentication-flow.html.
    void AWSCognitoAuthenticationProvider::InitiateAuthInternalAsync(const AZStd::string& username, const AZStd::string& password
        , AZStd::function<void(Aws::CognitoIdentityProvider::Model::InitiateAuthOutcome outcome)> outcomeCallback)
    {
        std::shared_ptr<Aws::CognitoIdentityProvider::CognitoIdentityProviderClient> cognitoIdentityProviderClient =
            AZ::Interface<IAWSClientAuthRequests>::Get()->GetCognitoIDPClient();

        AZ::JobContext* jobContext = nullptr;
        AWSCore::AWSCoreRequestBus::BroadcastResult(jobContext, &AWSCore::AWSCoreRequests::GetDefaultJobContext);

        AZ::Job* initiateAuthJob = AZ::CreateJobFunction([this, cognitoIdentityProviderClient, username, password, outcomeCallback]()
        {
            // Set Request parameters.
            Aws::CognitoIdentityProvider::Model::InitiateAuthRequest initiateAuthRequest;
            initiateAuthRequest.SetClientId(m_settings->m_appClientId.c_str());
            initiateAuthRequest.SetAuthFlow(Aws::CognitoIdentityProvider::Model::AuthFlowType::USER_PASSWORD_AUTH);

            // Set username and password for Password grant/ Initiate Auth flow.
            Aws::Map<Aws::String, Aws::String> authParameters
            {
                {COGNITO_USERNAME_KEY, username.c_str()},
                {COGNITO_PASSWORD_KEY, password.c_str()}
            };
            initiateAuthRequest.SetAuthParameters(authParameters);

            Aws::CognitoIdentityProvider::Model::InitiateAuthOutcome initiateAuthOutcome{ cognitoIdentityProviderClient->InitiateAuth(initiateAuthRequest) };
            outcomeCallback(initiateAuthOutcome);
        }, true, jobContext);
        initiateAuthJob->Start();
    }

    void AWSCognitoAuthenticationProvider::UpdateTokens(const Aws::CognitoIdentityProvider::Model::AuthenticationResultType& authenticationResult)
    {
        m_authenticationTokens = AuthenticationTokens(authenticationResult.GetAccessToken().c_str(), authenticationResult.GetRefreshToken().c_str(),
            authenticationResult.GetIdToken().c_str(), ProviderNameEnum::AWSCognitoIDP,
            authenticationResult.GetExpiresIn());
    }
} // namespace AWSClientAuth