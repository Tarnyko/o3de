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

#pragma once

#include <AzTest/AzTest.h>
#include <AzTest/Utils.h>
#include <AzFramework/IO/LocalFileIO.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <AzCore/Memory/PoolAllocator.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/Settings/SettingsRegistry.h>
#include <AzCore/Settings/SettingsRegistryImpl.h>
#include <AzCore/Serialization/Json/RegistrationContext.h>
#include <AzCore/Serialization/Json/JsonSystemComponent.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Jobs/JobManager.h>
#include <AzCore/Jobs/JobManagerBus.h>
#include <AzCore/Jobs/JobContext.h>
#include <Authentication/AWSCognitoAuthenticationProvider.h>
#include <Authentication/LWAAuthenticationProvider.h>
#include <Authentication/AuthenticationProviderBus.h>
#include <Authorization/AWSCognitoAuthorizationBus.h>
#include <UserManagement/AWSCognitoUserManagementBus.h>
#include <AWSCoreBus.h>
#include <AWSClientAuthBus.h>
#include <AWSNativeSDKInit/AWSNativeSDKInit.h>
#include <HttpRequestor/HttpRequestorBus.h>

#include <aws/core/utils/Outcome.h>
#include <aws/cognito-idp/model/InitiateAuthRequest.h>
#include <aws/cognito-idp/model/InitiateAuthResult.h>
#include <aws/cognito-idp/CognitoIdentityProviderClient.h>
#include <aws/cognito-idp/model/SignUpRequest.h>
#include <aws/cognito-idp/model/SignUpResult.h>
#include <aws/cognito-idp/model/ConfirmSignUpRequest.h>
#include <aws/cognito-idp/model/ConfirmSignUpResult.h>
#include <aws/cognito-idp/model/RespondToAuthChallengeRequest.h>
#include <aws/cognito-idp/model/RespondToAuthChallengeResult.h>
#include <aws/cognito-idp/model/ForgotPasswordResult.h>
#include <aws/cognito-idp/model/ForgotPasswordRequest.h>
#include <aws/cognito-idp/model/ConfirmForgotPasswordRequest.h>
#include <aws/cognito-idp/model/ConfirmForgotPasswordResult.h>
#include <aws/cognito-idp/model/SetUserMFAPreferenceRequest.h>
#include <aws/cognito-idp/model/SetUserMFAPreferenceResult.h>


#include <aws/cognito-identity/CognitoIdentityClient.h>
#include <aws/cognito-identity/model/GetCredentialsForIdentityRequest.h>
#include <aws/cognito-identity/model/GetCredentialsForIdentityResult.h>
#include <aws/cognito-identity/model/GetIdRequest.h>
#include <aws/cognito-identity/model/GetIdResult.h>



namespace AWSClientAuthUnitTest
{
    constexpr char TEST_USERNAME[] = "TestUsername";
    constexpr char TEST_PASSWORD[] = "TestPassword";
    constexpr char TEST_NEW_PASSWORD[] = "TestNewPassword";
    constexpr char TEST_CODE[] = "TestCode";
    constexpr char TEST_EMAIL[] = "test@test.com";
    constexpr char TEST_PHONE[] = "+11234567890";
    constexpr char TEST_COGNITO_CLIENTID[] = "TestCognitoClientId";
    constexpr char TEST_EXCEPTION[] = "TestException";
    constexpr char TEST_SESSION[] = "TestSession";
    constexpr char TEST_TOKEN[] = "TestToken";
    constexpr char TEST_ACCOUNT_ID[] = "1234567890";
    constexpr char TEST_IDENTITY_POOL_ID[] = "TestIdenitityPoolId";
    constexpr char TEST_IDENTITY_ID[] = "TestIdenitityId";
    constexpr char TEST_ACCESS_TOKEN[] = "TestAccessToken";
    constexpr char TEST_REFRESH_TOKEN[] = "TestRefreshToken";
    constexpr char TEST_ID_TOKEN[] = "TestIdToken";
    constexpr char TEST_ACCESS_KEY_ID[] = "TestAccessKeyId";
    constexpr char TEST_SECRET_KEY_ID[] = "TestSecretKeyId";

    class HttpRequestorRequestBusMock
        : public HttpRequestor::HttpRequestorRequestBus::Handler
    {
    public:
        HttpRequestorRequestBusMock()
        {
            ON_CALL(*this, AddRequestWithHeadersAndBody(testing::_, testing::_, testing::_, testing::_, testing::_)).WillByDefault(testing::Invoke(this, &HttpRequestorRequestBusMock::AddRequestWithHeadersAndBodyMock));
            HttpRequestor::HttpRequestorRequestBus::Handler::BusConnect();
        }

        virtual ~HttpRequestorRequestBusMock()
        {
            HttpRequestor::HttpRequestorRequestBus::Handler::BusDisconnect();
        }

        MOCK_METHOD5(AddRequestWithHeadersAndBody, void(const AZStd::string& URI, Aws::Http::HttpMethod method, const HttpRequestor::Headers& headers, const AZStd::string& body, const HttpRequestor::Callback& callback));

        void AddRequestWithHeadersAndBodyError(const AZStd::string& URI, Aws::Http::HttpMethod method, const HttpRequestor::Headers& headers, const AZStd::string& body, const HttpRequestor::Callback& callback)
        {
            AZ_UNUSED(URI);
            AZ_UNUSED(method);
            AZ_UNUSED(headers);
            AZ_UNUSED(body);

            Aws::Utils::Json::JsonValue jsonValue;
            jsonValue.WithString("error", "TestError");
            Aws::Utils::Json::JsonView jsonView(jsonValue);
            Aws::Http::HttpResponseCode code = Aws::Http::HttpResponseCode::INTERNAL_SERVER_ERROR;
            callback(jsonView, code);
        }

        void AddRequest(const AZStd::string& URI, Aws::Http::HttpMethod method, const HttpRequestor::Callback& callback) override
        {
            AZ_UNUSED(URI);
            AZ_UNUSED(method);
            AZ_UNUSED(callback);
        }

        void AddRequestWithHeaders(const AZStd::string& URI, Aws::Http::HttpMethod method, const HttpRequestor::Headers& headers, const HttpRequestor::Callback& callback) override
        {
            AZ_UNUSED(URI);
            AZ_UNUSED(method);
            AZ_UNUSED(headers);
            AZ_UNUSED(callback);
        }
        void AddRequestWithHeadersAndBodyMock(const AZStd::string& URI, Aws::Http::HttpMethod method, const HttpRequestor::Headers& headers, const AZStd::string& body, const HttpRequestor::Callback& callback)
        {
            AZ_UNUSED(URI);
            AZ_UNUSED(method);
            AZ_UNUSED(headers);
            AZ_UNUSED(body);

            Aws::Utils::Json::JsonValue jsonValue;
            jsonValue.WithString("user_code", "TestCode");
            jsonValue.WithString("device_code", "TestDeviceCode");
            jsonValue.WithString("verification_uri", "TestVerificationURI");
            jsonValue.WithString("access_token", "TestAccessToken");
            jsonValue.WithString("refresh_token", "TestRefreshToken");
            jsonValue.WithString("id_token", "TestIdToken");
            jsonValue.WithString("expires_in", "600");
            Aws::Utils::Json::JsonView jsonView(jsonValue);
            Aws::Http::HttpResponseCode code = Aws::Http::HttpResponseCode::OK;
            callback(jsonView, code);
        }

        void AddTextRequest(const AZStd::string& URI, Aws::Http::HttpMethod method, const HttpRequestor::TextCallback& callback) override
        {
            AZ_UNUSED(URI);
            AZ_UNUSED(method);
            AZ_UNUSED(callback);
        }
        void AddTextRequestWithHeaders(const AZStd::string& URI, Aws::Http::HttpMethod method, const HttpRequestor::Headers& headers, const HttpRequestor::TextCallback& callback) override
        {
            AZ_UNUSED(URI);
            AZ_UNUSED(method);
            AZ_UNUSED(headers);
            AZ_UNUSED(callback);
        }
        void AddTextRequestWithHeadersAndBody(const AZStd::string& URI, Aws::Http::HttpMethod method, const HttpRequestor::Headers& headers, const AZStd::string& body, const HttpRequestor::TextCallback& callback) override
        {
            AZ_UNUSED(URI);
            AZ_UNUSED(method);
            AZ_UNUSED(headers);
            AZ_UNUSED(body);
            AZ_UNUSED(callback);
        }
    };

    class CognitoIdentityProviderClientMock
        : public Aws::CognitoIdentityProvider::CognitoIdentityProviderClient
    {
    public:

        CognitoIdentityProviderClientMock() : Aws::CognitoIdentityProvider::CognitoIdentityProviderClient(Aws::Auth::AWSCredentials())
        {
            ON_CALL(*this, InitiateAuth(testing::_)).WillByDefault(testing::Invoke(this, &CognitoIdentityProviderClientMock::InitiateAuthMock));
            ON_CALL(*this, SignUp(testing::_)).WillByDefault(testing::Invoke(this, &CognitoIdentityProviderClientMock::SignUpMock));
            ON_CALL(*this, ConfirmSignUp(testing::_)).WillByDefault(testing::Invoke(this, &CognitoIdentityProviderClientMock::ConfirmSignUpMock));
            ON_CALL(*this, RespondToAuthChallenge(testing::_)).WillByDefault(testing::Invoke(this, &CognitoIdentityProviderClientMock::RespondToAuthChallengeMock));
            ON_CALL(*this, ForgotPassword(testing::_)).WillByDefault(testing::Invoke(this, &CognitoIdentityProviderClientMock::ForgotPasswordMock));
            ON_CALL(*this, ConfirmForgotPassword(testing::_)).WillByDefault(testing::Invoke(this, &CognitoIdentityProviderClientMock::ConfirmForgotPasswordMock));
            ON_CALL(*this, SetUserMFAPreference(testing::_)).WillByDefault(testing::Invoke(this, &CognitoIdentityProviderClientMock::SetUserMFAPreferenceMock));
        }

        MOCK_CONST_METHOD1(InitiateAuth
            , Aws::CognitoIdentityProvider::Model::InitiateAuthOutcome(const Aws::CognitoIdentityProvider::Model::InitiateAuthRequest& request));
        MOCK_CONST_METHOD1(RespondToAuthChallenge
            , Aws::CognitoIdentityProvider::Model::RespondToAuthChallengeOutcome(const Aws::CognitoIdentityProvider::Model::RespondToAuthChallengeRequest& request));
        MOCK_CONST_METHOD1(SignUp
            , Aws::CognitoIdentityProvider::Model::SignUpOutcome(const Aws::CognitoIdentityProvider::Model::SignUpRequest& request));
        MOCK_CONST_METHOD1(ConfirmSignUp
            , Aws::CognitoIdentityProvider::Model::ConfirmSignUpOutcome(const Aws::CognitoIdentityProvider::Model::ConfirmSignUpRequest& request));
        MOCK_CONST_METHOD1(ForgotPassword
            , Aws::CognitoIdentityProvider::Model::ForgotPasswordOutcome(const Aws::CognitoIdentityProvider::Model::ForgotPasswordRequest& request));
        MOCK_CONST_METHOD1(ConfirmForgotPassword
            , Aws::CognitoIdentityProvider::Model::ConfirmForgotPasswordOutcome(const Aws::CognitoIdentityProvider::Model::ConfirmForgotPasswordRequest& request));
        MOCK_CONST_METHOD1(SetUserMFAPreference
            , Aws::CognitoIdentityProvider::Model::SetUserMFAPreferenceOutcome(const Aws::CognitoIdentityProvider::Model::SetUserMFAPreferenceRequest& request));


        Aws::CognitoIdentityProvider::Model::InitiateAuthOutcome InitiateAuthMock(const Aws::CognitoIdentityProvider::Model::InitiateAuthRequest& request)
        {
            AZ_UNUSED(request);
            Aws::CognitoIdentityProvider::Model::AuthenticationResultType authenticationResult;
            authenticationResult.SetAccessToken(TEST_ACCESS_TOKEN);
            authenticationResult.SetRefreshToken(TEST_REFRESH_TOKEN);
            authenticationResult.SetIdToken(TEST_ID_TOKEN);
            authenticationResult.SetExpiresIn(5);
            Aws::CognitoIdentityProvider::Model::InitiateAuthResult result;
            result.SetChallengeName(Aws::CognitoIdentityProvider::Model::ChallengeNameType::NOT_SET);
            result.SetAuthenticationResult(authenticationResult);
            Aws::CognitoIdentityProvider::Model::InitiateAuthOutcome outcome(result);
            return outcome;
        }

        Aws::CognitoIdentityProvider::Model::SignUpOutcome SignUpMock(const Aws::CognitoIdentityProvider::Model::SignUpRequest& request)
        {
            AZ_UNUSED(request);
            Aws::CognitoIdentityProvider::Model::SignUpResult result;
            result.SetUserSub("TestUserUUID");
            Aws::CognitoIdentityProvider::Model::SignUpOutcome outcome(result);
            return outcome;
        }

        Aws::CognitoIdentityProvider::Model::RespondToAuthChallengeOutcome RespondToAuthChallengeMock(const Aws::CognitoIdentityProvider::Model::RespondToAuthChallengeRequest& request)
        {
            AZ_UNUSED(request);
            Aws::CognitoIdentityProvider::Model::RespondToAuthChallengeResult result;
            Aws::CognitoIdentityProvider::Model::AuthenticationResultType authenticationResult;
            authenticationResult.SetAccessToken(TEST_ACCESS_TOKEN);
            authenticationResult.SetRefreshToken(TEST_REFRESH_TOKEN);
            authenticationResult.SetIdToken(TEST_ID_TOKEN);
            authenticationResult.SetExpiresIn(30);
            result.SetAuthenticationResult(authenticationResult);
            Aws::CognitoIdentityProvider::Model::RespondToAuthChallengeOutcome outcome(result);
            return outcome;
        }

        Aws::CognitoIdentityProvider::Model::ConfirmSignUpOutcome ConfirmSignUpMock(const Aws::CognitoIdentityProvider::Model::ConfirmSignUpRequest& request)
        {
            AZ_UNUSED(request);
            Aws::CognitoIdentityProvider::Model::ConfirmSignUpResult result;
            Aws::CognitoIdentityProvider::Model::ConfirmSignUpOutcome outcome(result);
            return outcome;
        }

        Aws::CognitoIdentityProvider::Model::ForgotPasswordOutcome ForgotPasswordMock(const Aws::CognitoIdentityProvider::Model::ForgotPasswordRequest& request)
        {
            AZ_UNUSED(request);
            Aws::CognitoIdentityProvider::Model::ForgotPasswordResult result;
            Aws::CognitoIdentityProvider::Model::ForgotPasswordOutcome outcome(result);
            return outcome;
        }

        Aws::CognitoIdentityProvider::Model::ConfirmForgotPasswordOutcome ConfirmForgotPasswordMock(const Aws::CognitoIdentityProvider::Model::ConfirmForgotPasswordRequest& request)
        {
            AZ_UNUSED(request);
            Aws::CognitoIdentityProvider::Model::ConfirmForgotPasswordResult result;
            Aws::CognitoIdentityProvider::Model::ConfirmForgotPasswordOutcome outcome(result);
            return outcome;
        }
        
        Aws::CognitoIdentityProvider::Model::SetUserMFAPreferenceOutcome SetUserMFAPreferenceMock(const Aws::CognitoIdentityProvider::Model::SetUserMFAPreferenceRequest& request)
        {
            AZ_UNUSED(request);
            Aws::CognitoIdentityProvider::Model::SetUserMFAPreferenceResult result;
            Aws::CognitoIdentityProvider::Model::SetUserMFAPreferenceOutcome outcome(result);
            return outcome;
        }
    };

    class CognitoIdentityClientMock
        : public Aws::CognitoIdentity::CognitoIdentityClient
    {
    public:
        CognitoIdentityClientMock()
        {
            ON_CALL(*this, GetId(testing::_)).WillByDefault(testing::Invoke(this, &CognitoIdentityClientMock::GetIdMock));
            ON_CALL(*this, GetCredentialsForIdentity(testing::_)).WillByDefault(testing::Invoke(this, &CognitoIdentityClientMock::GetCredentialsForIdentityMock));
        }

        MOCK_CONST_METHOD1(GetId
            , Aws::CognitoIdentity::Model::GetIdOutcome(const Aws::CognitoIdentity::Model::GetIdRequest& request));
        MOCK_CONST_METHOD1(GetCredentialsForIdentity
            , Aws::CognitoIdentity::Model::GetCredentialsForIdentityOutcome(const Aws::CognitoIdentity::Model::GetCredentialsForIdentityRequest& request));


        Aws::CognitoIdentity::Model::GetIdOutcome GetIdMock(const Aws::CognitoIdentity::Model::GetIdRequest& request)
        {
            AZ_UNUSED(request);
            Aws::CognitoIdentity::Model::GetIdResult result;
            result.SetIdentityId(TEST_IDENTITY_ID);
            Aws::CognitoIdentity::Model::GetIdOutcome outcome(result);
            return outcome;
        }

        Aws::CognitoIdentity::Model::GetCredentialsForIdentityOutcome GetCredentialsForIdentityMock(const Aws::CognitoIdentity::Model::GetCredentialsForIdentityRequest& request)
        {
            AZ_UNUSED(request);
            Aws::CognitoIdentity::Model::Credentials creds;
            creds.SetAccessKeyId(TEST_ACCESS_KEY_ID);
            creds.SetSecretKey(TEST_SECRET_KEY_ID);
            creds.SetExpiration(Aws::Utils::DateTime(std::chrono::system_clock::now() + std::chrono::seconds(600)));
            Aws::CognitoIdentity::Model::GetCredentialsForIdentityResult result;
            result.SetIdentityId(TEST_IDENTITY_ID);
            result.SetCredentials(creds);
            Aws::CognitoIdentity::Model::GetCredentialsForIdentityOutcome outcome(result);
            return outcome;
        }
    };

    class AuthenticationProviderMock
        : public AWSClientAuth::AuthenticationProviderInterface
    {
    public:

        AuthenticationProviderMock()
        {
            ON_CALL(*this, Initialize(testing::_)).WillByDefault(testing::Return(true));
        }

        virtual ~AuthenticationProviderMock() = default;

        MOCK_METHOD1(Initialize, bool(AZStd::weak_ptr<AZ::SettingsRegistryInterface> settingsRegistry));
        MOCK_METHOD2(PasswordGrantSingleFactorSignInAsync, void(const AZStd::string& username, const AZStd::string& password));
        MOCK_METHOD2(PasswordGrantMultiFactorSignInAsync, void(const AZStd::string& username, const AZStd::string& password));
        MOCK_METHOD2(PasswordGrantMultiFactorConfirmSignInAsync, void(const AZStd::string& username, const AZStd::string& confirmationCode));
        MOCK_METHOD0(DeviceCodeGrantSignInAsync, void());
        MOCK_METHOD0(DeviceCodeGrantConfirmSignInAsync, void());
        MOCK_METHOD0(RefreshTokensAsync, void());
        MOCK_METHOD0(GetAuthenticationTokens, AWSClientAuth::AuthenticationTokens());
        MOCK_METHOD0(SignOut, void());
    };

    class AuthenticationProviderNotificationsBusMock
        : public AWSClientAuth::AuthenticationProviderNotificationBus::Handler
    {
    public:
        AuthenticationProviderNotificationsBusMock()
        {
            AWSClientAuth::AuthenticationProviderNotificationBus::Handler::BusConnect();
        }

        virtual ~AuthenticationProviderNotificationsBusMock()
        {
            AWSClientAuth::AuthenticationProviderNotificationBus::Handler::BusDisconnect();
        }

        MOCK_METHOD1(OnPasswordGrantSingleFactorSignInSuccess, void(const AWSClientAuth::AuthenticationTokens& authenticationToken));
        MOCK_METHOD1(OnPasswordGrantSingleFactorSignInFail, void(const AZStd::string& error));
        MOCK_METHOD0(OnPasswordGrantMultiFactorSignInSuccess, void());
        MOCK_METHOD1(OnPasswordGrantMultiFactorSignInFail, void(const AZStd::string& error));
        MOCK_METHOD1(OnPasswordGrantMultiFactorConfirmSignInSuccess, void(const AWSClientAuth::AuthenticationTokens& authenticationToken));
        MOCK_METHOD1(OnPasswordGrantMultiFactorConfirmSignInFail, void(const AZStd::string& error));
        MOCK_METHOD3(OnDeviceCodeGrantSignInSuccess, void(const AZStd::string& userCode, const AZStd::string& verificationUrl, const int codeExpiresInSeconds));
        MOCK_METHOD1(OnDeviceCodeGrantSignInFail, void(const AZStd::string& error));
        MOCK_METHOD1(OnDeviceCodeGrantConfirmSignInSuccess, void(const AWSClientAuth::AuthenticationTokens& authenticationToken));
        MOCK_METHOD1(OnDeviceCodeGrantConfirmSignInFail, void(const AZStd::string& error));
        MOCK_METHOD1(OnRefreshTokensSuccess, void(const AWSClientAuth::AuthenticationTokens& authenticationToken));
        MOCK_METHOD1(OnRefreshTokensFail, void(const AZStd::string& error));
        MOCK_METHOD1(OnSignOut, void(const AWSClientAuth::ProviderNameEnum& providerName));
    };

    class AWSCognitoAuthorizationNotificationsBusMock
        : public AWSClientAuth::AWSCognitoAuthorizationNotificationBus::Handler
    {
    public:
        AWSCognitoAuthorizationNotificationsBusMock()
        {
            AWSClientAuth::AWSCognitoAuthorizationNotificationBus::Handler::BusConnect();
        }

        virtual ~AWSCognitoAuthorizationNotificationsBusMock()
        {
            AWSClientAuth::AWSCognitoAuthorizationNotificationBus::Handler::BusDisconnect();
        }

        MOCK_METHOD1(OnRequestAWSCredentialsSuccess, void(const AWSClientAuth::ClientAuthAWSCredentials& awsCredentials));
        MOCK_METHOD1(OnRequestAWSCredentialsFail, void(const AZStd::string& error));
    };

    class AWSCognitoUserManagementNotificationsBusMock
        : public AWSClientAuth::AWSCognitoUserManagementNotificationBus::Handler
    {
    public:
        AWSCognitoUserManagementNotificationsBusMock()
        {
            AWSClientAuth::AWSCognitoUserManagementNotificationBus::Handler::BusConnect();
        }

        virtual ~AWSCognitoUserManagementNotificationsBusMock()
        {
            AWSClientAuth::AWSCognitoUserManagementNotificationBus::Handler::BusDisconnect();
        }

        MOCK_METHOD1(OnEmailSignUpSuccess, void(const AZStd::string& uuid));
        MOCK_METHOD1(OnEmailSignUpFail, void(const AZStd::string& error));
        MOCK_METHOD1(OnPhoneSignUpSuccess, void(const AZStd::string& uuid));
        MOCK_METHOD1(OnPhoneSignUpFail, void(const AZStd::string& error));
        MOCK_METHOD0(OnConfirmSignUpSuccess, void());
        MOCK_METHOD1(OnConfirmSignUpFail, void(const AZStd::string& error));
        MOCK_METHOD0(OnForgotPasswordSuccess, void());
        MOCK_METHOD1(OnForgotPasswordFail, void(const AZStd::string& error));
        MOCK_METHOD0(OnConfirmForgotPasswordSuccess, void());
        MOCK_METHOD1(OnConfirmForgotPasswordFail, void(const AZStd::string& error));
        MOCK_METHOD0(OnEnableMFASuccess, void());
        MOCK_METHOD1(OnEnableMFAFail, void(const AZStd::string& error));
    };

    class AWSClientAuthGemAllocatorFixture
        : public UnitTest::ScopedAllocatorSetupFixture
        , public AZ::ComponentApplicationBus::Handler
        , public AWSClientAuth::AWSClientAuthRequestBus::Handler
    {
    public:

        AWSClientAuthGemAllocatorFixture()
        {

        }

        AWSClientAuthGemAllocatorFixture(bool connectClientAuthBus)
        {
            m_connectClientAuthBus = connectClientAuthBus;
        }

        virtual ~AWSClientAuthGemAllocatorFixture() = default;

    protected:
        AZStd::shared_ptr<AZ::SerializeContext> m_serializeContext;
        AZStd::unique_ptr<AZ::JsonRegistrationContext> m_registrationContext;
        AZStd::shared_ptr<AZ::SettingsRegistryImpl> m_settingsRegistry;
        AZStd::unique_ptr<AZStd::string> m_testFolder;
        bool m_testFolderCreated = false;
        AZStd::unique_ptr<AZ::JobContext> m_jobContext;
        AZStd::unique_ptr<AZ::JobCancelGroup> m_jobCancelGroup;
        AZStd::unique_ptr<AZ::JobManager> m_jobManager;

        std::shared_ptr<CognitoIdentityProviderClientMock> m_cognitoIdentityProviderClientMock;
        std::shared_ptr<CognitoIdentityClientMock> m_cognitoIdentityClientMock;
        bool m_connectClientAuthBus = true;

        AuthenticationProviderNotificationsBusMock m_authenticationProviderNotificationsBusMock;
        AWSCognitoAuthorizationNotificationsBusMock m_awsCognitoAuthorizationNotificationsBusMock;
        AWSCognitoUserManagementNotificationsBusMock m_awsCognitoUserManagementNotificationsBusMock;

        void SetUp() override
        {
            AZ::AllocatorInstance<AZ::ThreadPoolAllocator>::Create();

            AZ::IO::FileIOBase::SetInstance(aznew AZ::IO::LocalFileIO());

            m_serializeContext = AZStd::make_unique<AZ::SerializeContext>();
            m_registrationContext = AZStd::make_unique<AZ::JsonRegistrationContext>();

            AZ::JsonSystemComponent::Reflect(m_registrationContext.get());

            m_settingsRegistry = AZStd::make_unique<AZ::SettingsRegistryImpl>();

            m_settingsRegistry->SetContext(m_serializeContext.get());
            m_settingsRegistry->SetContext(m_registrationContext.get());

            AZ::ComponentApplicationBus::Handler::BusConnect();

            if (m_connectClientAuthBus)
            {
                AZ::Interface<IAWSClientAuthRequests>::Register(this);
                AWSClientAuth::AWSClientAuthRequestBus::Handler::BusConnect();
            }

            m_testFolder = AZStd::make_unique<AZStd::string>("AWSClientAuthTest_");
            m_testFolder->append(AZ::Uuid::CreateRandom().ToString<AZStd::string>(false, false));

            AZ::JobManagerDesc jobManagerDesc;
            AZ::JobManagerThreadDesc threadDesc;

            m_jobManager.reset(aznew AZ::JobManager(jobManagerDesc));
            m_jobCancelGroup.reset(aznew AZ::JobCancelGroup());
            jobManagerDesc.m_workerThreads.push_back(threadDesc);
            jobManagerDesc.m_workerThreads.push_back(threadDesc);
            jobManagerDesc.m_workerThreads.push_back(threadDesc);
            m_jobContext.reset(aznew AZ::JobContext(*m_jobManager, *m_jobCancelGroup));
            AZ::JobContext::SetGlobalContext(m_jobContext.get());

            AWSNativeSDKInit::InitializationManager::InitAwsApi();
            m_cognitoIdentityProviderClientMock = std::make_shared<CognitoIdentityProviderClientMock>();
            m_cognitoIdentityClientMock = std::make_shared<CognitoIdentityClientMock>();
        }

        void TearDown() override
        {
            AZ::JobContext::SetGlobalContext(nullptr);
            m_jobContext.reset();
            m_jobCancelGroup.reset();
            m_jobManager.reset();

            m_cognitoIdentityProviderClientMock.reset();
            m_cognitoIdentityClientMock.reset();

            AWSNativeSDKInit::InitializationManager::Shutdown();


            AZ::AllocatorInstance<AZ::ThreadPoolAllocator>::Destroy();

            if (m_testFolderCreated)
            {
                DeleteFolderRecursive(*m_testFolder);
            }

            m_registrationContext->EnableRemoveReflection();
            AZ::JsonSystemComponent::Reflect(m_registrationContext.get());
            m_registrationContext->DisableRemoveReflection();

            AZ::ComponentApplicationBus::Handler::BusDisconnect();
            if (m_connectClientAuthBus)
            {
               AZ::Interface<IAWSClientAuthRequests>::Unregister(this);
                AWSClientAuth::AWSClientAuthRequestBus::Handler::BusDisconnect();
            }

            m_testFolder.reset();
            m_settingsRegistry.reset();
            m_serializeContext.reset();
            m_registrationContext.reset();


            delete AZ::IO::FileIOBase::GetInstance();
            AZ::IO::FileIOBase::SetInstance(nullptr);

        }

        // ComponentApplicationBus overrides. Required by settings registry for json serialization context.
        AZ::ComponentApplication* GetApplication() override { return nullptr; }
        void RegisterComponentDescriptor(const AZ::ComponentDescriptor*) override { }
        void UnregisterComponentDescriptor(const AZ::ComponentDescriptor*) override { }
        bool AddEntity(AZ::Entity*) override { return true; }
        bool RemoveEntity(AZ::Entity*) override { return true; }
        bool DeleteEntity(const AZ::EntityId&) override { return true; }
        AZ::Entity* FindEntity(const AZ::EntityId&) override { return nullptr; }
        AZ::BehaviorContext* GetBehaviorContext() override { return nullptr; }
        const char* GetExecutableFolder() const override { return nullptr; }
        const char* GetAppRoot() const override { return nullptr; }
        AZ::Debug::DrillerManager* GetDrillerManager() override { return nullptr; }
        void EnumerateEntities(const EntityCallback& /*callback*/) override {}
        void QueryApplicationType(AZ::ApplicationTypeQuery& /*appType*/) const override {}
        AZ::SerializeContext* GetSerializeContext() override
        {
            return m_serializeContext.get();
        }

        AZ::JsonRegistrationContext* GetJsonRegistrationContext() override
        {
            return m_registrationContext.get();
        }

        // AWSClientAuthBus
        std::shared_ptr<Aws::CognitoIdentityProvider::CognitoIdentityProviderClient> GetCognitoIDPClient() override
        {
            return m_cognitoIdentityProviderClientMock;
        }

        std::shared_ptr<Aws::CognitoIdentity::CognitoIdentityClient> GetCognitoIdentityClient() override
        {
            return m_cognitoIdentityClientMock;
        }

        // TODO Add safety check. Also use pattern to create and remove one file.
        static void DeleteFolderRecursive(const AZStd::string& path)
        {
            auto callback = [&path](const char* filename, bool isFile) -> bool
            {
                if (isFile)
                {
                    AZStd::string filePath = path;
                    filePath += '/';
                    filePath += filename;
                    AZ::IO::SystemFile::Delete(filePath.c_str());
                }
                else
                {
                    if (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0)
                    {
                        AZStd::string folderPath = path;
                        folderPath += '/';
                        folderPath += filename;
                        DeleteFolderRecursive(folderPath);
                    }
                }
                return true;
            };
            AZStd::string searchPath = path;
            searchPath += "/*";
            AZ::IO::SystemFile::FindFiles(searchPath.c_str(), callback);
            AZ::IO::SystemFile::DeleteDir(path.c_str());
        }

        AZStd::string CreateTestFile(AZStd::string_view name, AZStd::string_view content)
        {
            using namespace AZ::IO;

            AZStd::string path = AZStd::string::format("%s/%s/%.*s", m_testFolder->c_str(),
                AZ::SettingsRegistryInterface::RegistryFolder, static_cast<int>(name.length()), name.data());

            SystemFile file;
            if (!file.Open(path.c_str(), SystemFile::OpenMode::SF_OPEN_CREATE | SystemFile::SF_OPEN_CREATE_PATH | SystemFile::SF_OPEN_WRITE_ONLY))
            {
                AZ_Assert(false, "Unable to open test file for writing: %s", path.c_str());
                return path;
            }

            if (file.Write(content.data(), content.size()) != content.size())
            {
                AZ_Assert(false, "Unable to write content to test file: %s", path.c_str());
            }

            m_testFolderCreated = true;
            return path;
        }

    };

    
}