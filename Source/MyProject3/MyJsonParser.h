#pragma once

#include "CoreMinimal.h"
#include "JsonUtilities.h"
#include "MyJsonParser.generated.h"

// FConversation 구조체 선언
USTRUCT(BlueprintType)
struct FConversation : public FTableRowBase
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Name;

    UPROPERTY(BlueprintReadWrite)
    FString Emotion;

    UPROPERTY(BlueprintReadWrite)
    FString Message;
};

// UMyJsonParser 클래스 선언
UCLASS(Blueprintable)
class MYPROJECT3_API UMyJsonParser : public UObject
{
    GENERATED_BODY()

public:
    // JSON 파일을 파싱하는 정적 함수 선언
    UFUNCTION(BlueprintCallable, Category = "JSON")
    void ParseJson();

    // 파싱된 대화 내용을 저장할 배열 선언 (블루프린트에서 읽기 가능)
    UPROPERTY(BlueprintReadOnly, Category = "JSON")
    TArray<FConversation> Conversations;
};
