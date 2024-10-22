
## 프로젝트 소개

사용자가 입력한 스토리대로 LLM이 캐릭터를 연기하여 장면을 만들어내는 콘텐츠 제작 툴입니다.

유튜브 채널 [K-현실고증](https://www.youtube.com/@Krealityshow)과 협업으로 개발했으며 사용자가 상황을 설정하면 LLM이 원작 캐릭터들의 정보를 바탕으로 원작 느낌을 살린 대본을 만들어냅니다.

![image.jpg1](https://github.com/lsh210726/K-RealitySimulator/blob/main/readmeImg/krsImg2.png) |![image.jpg2](https://github.com/lsh210726/K-RealitySimulator/blob/main/readmeImg/krsImg1.png)
--- | --- | 

https://www.youtube.com/watch?v=2QC24hAJ7NI

개발인원 : AI 1, 언리얼 1  
개발기간 : 24/7/23~24/9/10 (7주)  

2024 메타버스 개발자 경진대회 우수상 수상

---
### 개발한 것

 1. AI서버로 대본 생성 요청하는 Unreal Blueprint 커스텀 노드 구현
 2. AI서버에서 클라이언트의 요청을 받아 대본 작성 후 전달하는 기능 구현
 3. 스토리 작성, 대본 작성 AI 구현
---
### 사용기술
- 언리얼 엔진 5.4
- Unreal Blueprint
- C++
- Python
- GPT 4o
- FastAPI

## 언리얼-AI 아키텍처
![작가AI 아키텍처](https://github.com/lsh210726/K-RealitySimulator/blob/main/readmeImg/khsArch3.jpg)
## 언리얼-파이썬 인터페이스  
대화문 생성에 필요한 데이터를 챗봇에 전달하는 함수입니다. 

레벨에 배치된 캐릭터들의 정보, 유저가 입력한 스토리 설정 등의 정보가 전달됩니다.

<img src="https://github.com/lsh210726/K-RealitySimulator/blob/main/readmeImg/customNode.png" alt="커스텀노드" style="width: 40%;">  

입력 핀으로 전달받은 값을 json형식으로 새로 생성해 FastAPI 서버로 전송합니다.

<details>
<summary>코드</summary>

```c++
//파이썬 서버로 유저가 작성한 상황설정 및 레벨에 배치된 캐릭터 정보, 새 대화인지 여부, 만약 오래된 대화이면 이전 대화 기록, 유저가 생성한 새로운 캐릭터 정보 등을 전달한다.
FString UMyJsonParser::SendRequestAndGetResponse(FString Characters, FString Description, bool bIsNewStory, FString LastConversation, FString NewCharacters, FString NewCharacterDescriptions)
{
    LastResponse = "";

    // 요청사항 담은 json문 생성
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

    JsonObject->SetArrayField("characters", ParseCharactersArray(Characters));//레벨에 배치된 캐릭터 리스트

    JsonObject->SetStringField("description", Description);//유저가 작성한 스토리 설명

    JsonObject->SetBoolField("isNewStory", bIsNewStory);//새 스토리인지 여부

    JsonObject->SetStringField("lastConversation", LastConversation);//이전 대화 내역

    JsonObject->SetArrayField("newCharacters", ParseCharactersArray(NewCharacters));//유저가 추가한 새 캐릭터 이름

    JsonObject->SetArrayField("newCharacterDescriptions", ParseCharactersArray(NewCharacterDescriptions));//새 캐릭터의 정보


    FString RequestContent;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestContent);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    //요청 전달
    SendPostRequest(RequestContent);

    return LastResponse;
}
```
</details>

## 대사 자동 재생성
대사가 너무 짧은 경우 자동으로 재생성합니다.


#### 문제점  
간헐적으로 대화문의 길이가 매우 짧게 생성되는 문제가 있었습니다. 
#### 해결법


![다이어그램](https://github.com/lsh210726/K-RealitySimulator/blob/main/readmeImg/mermaid-diagram-2024-10-22-180213.png)


해당 문제를 해결하기 위하여 대화문의 최소 길이(700자)를 지정하고 해당 길이보다 짧게 생성되었을 경우 재생성을 최대 2회 요청하도록 하였습니다.  
만약 최대 횟수까지 재생성을 해도 대화문 최소 길이를 충족하지 못할 경우, 생성된 대화문 중 가장 길이가 긴 대화문을 전달하도록 하였습니다.

<details>
<summary>코드</summary>

 
```python
min_length = 700 #대본 최소 길이
max_attempts = 3 #최대 시도 횟수. 대본 최소 길이를 충족하지 못할 경우 최대 몇 회까지 재생성을 시도할 지 지정


attempt = 0 #생성 시도 횟수
responses = [] #답변들을 저장할 리스트

while attempt < max_attempts:
    try:
       
        # 응답에서 텍스트 추출
        answer = create_script(synopsis,situation.isNewStory,situation.lastConversation)
        responses.append(answer)


        # 응답 길이 확인
        if len(answer) >= min_length:
            return answer
        else:
            print(f"응답이 {min_length}자 미만입니다. 다시 시도합니다.")
            attempt += 1
    except Exception as e:
        print(f"오류 발생: {str(e)}")
        attempt += 1

# 모든 시도 실패 시 가장 긴 응답 반환
if responses:
    return max(responses, key=len)
else:
    return f"{max_attempts}번 시도했지만 {min_length}자 이상의 응답을 받지 못했습니다."

```

</details>


## 챗봇 출력 정형화

<img src="https://github.com/lsh210726/K-RealitySimulator/blob/main/readmeImg/khsFuncCall3.jpg" alt="functionCallingIMG" style="width: 80%;">  

챗봇으로부터 대사 뿐 아니라 대사를 말하는 캐릭터, 대사에 맞는 감정을 함께 출력시키고 출력 중 원하는 데이터들을 따로 처리해야 하기에 사전에 지정한 json 형식으로 출력되도록 하였습니다.

출력을 정형화하기 위해 OpenAI의 function calling 기능을 사용하여 출력 형식을 고정하였습니다.  
해당 기능을 사용하여도 ‘감정’ 부분의 출력이 정해진 감정 이외의 감정이 출력되는 문제가 있어 해당 경우 기본 감정으로 대체하도록 하였습니다.


## 회고
새롭게 캐릭터를 창작하는 것이 아닌 기존에 존재하는 콘텐츠 기반으로 챗봇을 구현해야 했기에 원작의 느낌을 살리는 것이 어려웠습니다.

캐릭터의 성격을 반영하고 원작의 느낌을 살리면서 스토리적인 재미도 챙기기 위해 노력했습니다.
