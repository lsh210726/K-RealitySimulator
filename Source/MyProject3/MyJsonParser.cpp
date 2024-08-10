#include "MyJsonParser.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Json.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "JsonObjectConverter.h"

// UMyJsonParser 클래스의 ParseJson 함수 정의
void UMyJsonParser::ParseJson()
{
    FString JsonString;
    // JSON 파일 경로 설정
    FString FilePath = FPaths::ProjectContentDir() / TEXT("Data/conversation.json");

    // 파일을 문자열로 로드
    if (FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        // JSON 문자열을 읽기 위한 JSON 리더 생성
        TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(JsonString);
        TSharedPtr<FJsonObject> JsonObject;

        // JSON 문자열을 JSON 오브젝트로 파싱
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            // "conversation" 배열 필드 가져오기
            const TArray<TSharedPtr<FJsonValue>>* JsonArray;
            if (JsonObject->TryGetArrayField(TEXT("conversation"), JsonArray))
            {
                // 기존의 대화 내용을 비우고 새로 파싱된 내용을 추가
                Conversations.Empty();

                // JSON 필드 이름을 변수로 저장
                FString NameField = TEXT("이름");
                FString EmotionField = TEXT("감정");
                FString MessageField = TEXT("대화내용");

                // JSON 배열을 반복하며 대화 객체를 파싱
                for (const TSharedPtr<FJsonValue>& Value : *JsonArray)
                {
                    TSharedPtr<FJsonObject> ConversationObject = Value->AsObject();
                    if (ConversationObject.IsValid())
                    {
                        FConversation Conversation;
                        // 각 필드를 구조체에 저장
                        Conversation.Name = ConversationObject->GetStringField(NameField);
                        Conversation.Emotion = ConversationObject->GetStringField(EmotionField);
                        Conversation.Message = ConversationObject->GetStringField(MessageField);

                        // 구조체를 배열에 추가
                        Conversations.Add(Conversation);
                    }
                }

                // 대화 내용을 로그로 출력
                for (const FConversation& Conv : Conversations)
                {
                    UE_LOG(LogTemp, Log, TEXT("Name: %s, Emotion: %s, Message: %s"), *Conv.Name, *Conv.Emotion, *Conv.Message);
                }
            }
        }
    }
    else
    {
        // 파일 로드 실패 시 오류 로그 출력
        UE_LOG(LogTemp, Error, TEXT("Failed to load JSON file."));
    }
}

void UMyJsonParser::SendPostRequest(FString RequestContent)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &UMyJsonParser::OnResponseReceived);
    Request->SetURL(TEXT("http://127.0.0.1:8000/generate-situation/"));//http://220.76.170.229:8000/generate-situation/
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(RequestContent);
    Request->ProcessRequest();
}

void UMyJsonParser::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    FString ResponseString;
    if (bWasSuccessful && Response.IsValid())
    {
        ResponseString = Response->GetContentAsString();
    }
    else
    {
        ResponseString = TEXT("Request failed");
    }

    // Handle the response here
    UE_LOG(LogTemp, Log, TEXT("Response: %s"), *ResponseString);

    // If you need to return the response as a string, you might want to store it in a class member
    LastResponse = ResponseString;
}

FString UMyJsonParser::SendRequestAndGetResponse(FString Characters, FString Description, bool bIsNewStory, FString LastConversation)
{
    LastResponse = "";
    // Create JSON request body
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetArrayField("characters", ParseCharactersArray(Characters));
    JsonObject->SetStringField("description", Description);
    JsonObject->SetBoolField("isNewStory", bIsNewStory);
    JsonObject->SetStringField("lastConversation", LastConversation);

    FString RequestContent;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestContent);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // Send the request
    SendPostRequest(RequestContent);

    // Wait for response (Note: In an actual implementation, you would handle the response asynchronously)
    // For simplicity, assume we have the response synchronously
    return LastResponse;
}

TArray<TSharedPtr<FJsonValue>> UMyJsonParser::ParseCharactersArray(FString Characters)
{
    TArray<TSharedPtr<FJsonValue>> CharactersArray;
    TArray<FString> CharactersList;
    Characters.ParseIntoArray(CharactersList, TEXT(","), true);

    for (FString& Character : CharactersList)
    {
        CharactersArray.Add(MakeShareable(new FJsonValueString(Character)));
    }

    return CharactersArray;
}
