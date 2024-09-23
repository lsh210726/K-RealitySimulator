
## 프로젝트 소개

사용자가 입력한 스토리대로 LLM이 캐릭터를 연기하여 장면을 만들어내는 콘텐츠 제작 툴입니다.

![image.jpg1](https://github.com/lsh210726/K-RealitySimulator/blob/main/readmeImg/krsImg2.png) |![image.jpg2](https://github.com/lsh210726/K-RealitySimulator/blob/main/readmeImg/krsImg1.png)
--- | --- | 

https://www.youtube.com/watch?v=2QC24hAJ7NI

개발인원 : AI 1, 언리얼 1  
개발기간 : 24/7/23~24/9/10 (7주)  
2024 메타버스 개발자 경진대회 우수상 수상

---
### 개발한 것
팀장을 맡아 프로젝트를 기획하고 총괄했습니다.  

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
![작가AI 아키텍처](https://github.com/lsh210726/K-RealitySimulator/blob/main/readmeImg/krsArch.jpg)
## 언리얼-파이썬 인터페이스
언리얼 클라이언트에서 캐릭터 정보, 유저가 입력한 스토리 설명, 기존 대화 내역 등을 파이썬 서버로 전달하는 커스텀 노드입니다.  

![커스텀노드](https://github.com/lsh210726/K-RealitySimulator/blob/main/readmeImg/krsCustomNode1.png)

입력 핀으로 전달받은 값을 json형식으로 새로 생성해 FastAPI 서버로 전송합니다.
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

    UE_LOG(LogTemp, Log, TEXT("Response: %s"), *LastConversation);
    //요청 전달
    SendPostRequest(RequestContent);

    return LastResponse;
}
```


## 파이썬 서버 요청 처리
클라이언트에서 받은 정보를 바탕으로 프롬프트를 작성해 GPT에 요청합니다. 

간헐적으로 대화문의 길이가 매우 짧게 생성되는 문제가 있었습니다. 해당 문제를 해결하기 위하여 대화문의 최소 길이(700자)를 지정하고 해당 길이보다 짧게 생성되었을 경우 재생성을 최대 2회 요청하도록 하였습니다.  
만약 최대 횟수까지 재생성을 해도 대화문 최소 길이를 충족하지 못할 경우 생성된 대화문 중 가장 길이가 긴 대화문을 전달하도록 하였습니다.
```python
@app.post("/generate-situation/")
async def generate_situation(situation: Situation):
    global prompt_persona #등장 캐릭터 페르소나 저장 문자열
    prompt_persona=""
    global newCharactersPersona #플레이어 생성 캐릭터 페르소나 저장 문자열
    newCharactersPersona = ""


#캐릭터 리스트에서 플레이어가 선택한 캐릭터들만 골라서 문자열에 더함
    for character in situation.characters:
        if character in personas:
            prompt_persona=prompt_persona+personas[character]
   
#플레이어가 생성한 캐릭터 이름과 특징을 합쳐서 하나의 페르소나로 만든 후 문자열에 더함
    newCharactersPersona = "\n".join([f"\n이름 : {name}\n특징 : {desc}\n" for name, desc in zip(situation.newCharacters, situation.newCharacterDescriptions)])
           
    #시놉시스 만들기
    synopsis = create_synopsis(situation.description)


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
## function calling 출력 정형화
LLM의 출력을 언리얼 클라이언트에서 사용하기 위해선 정해진 형식으로 출력을 정형화하는것이 필수입니다.

출력을 정형화하기 위해 OpenAI의 function calling 기능을 사용하여 출력 형식을 고정하였습니다.  
해당 기능을 사용하여도 ‘감정’ 부분의 출력이 정해진 감정 이외의 감정이 출력되는 문제가 있어 해당 경우 기본값 감정으로 대체하도록 하였습니다.
```python
#출력의 형태를 원하는 json형태로 고정시키기 function calling
function_description = {
    "name": "format_conversation",
    "description": "Create dialog or monologues between characters",
    "parameters": {
        "type": "object",
        "properties": {
            "conversation": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "이름": {"type": "string"},
                        "감정": {
                            "type": "string",
                            "enum": ["angry", "bored", "embarrassed", "happy", "neutral", "sad"]
                        },
                        "대화내용": {"type": "string"}
                    },
                    "required": ["이름", "감정", "대화내용"]
                }
            }
        },
        "required": ["conversation"]
    }
}

```

## 스토리 시놉시스 생성
사용자의 추상적인 요청으로 바로 대화문을 생성하니 스토리가 너무 평면적이고 단순하게 생성되는 문제가 있었습니다.  

사용자의 입력으로 짧은 씬(Scene)의 시놉시스를 생성 후 해당 시놉시스를 바탕으로 대화문을 생성하여 더 풍부하고 기승전결을 가진 대화문이 생성되었습니다.
```python
#스토리 시놉시스 생성
def create_synopsis(scene_description):
    response = client.chat.completions.create(
    model="gpt-4o-mini",
    max_tokens=3000,
    temperature=1.1,
    top_p=0.8,
    messages=[
            {"role": "system", "content": "You are a talented teenage drama story writer. The drama is about the school life of Korean high school students. Write a synopsis for a scene that the user requests. A scene has a consistent place and a continuous time. Write in English."},
            {"role": "user", "content": f"""
{scene_description}
Write a synopsis for the above scene. Refer to the character information given below.


{prompt_persona}


{newCharactersPersona}
"""}
        ]
    )


    print(response.choices[0].message.content)
    return response.choices[0].message.content
```
## 대화문 생성
생성된 시놉시스, 캐릭터 페르소나, 기존 대화 내역을 바탕으로 대화문을 생성합니다.
```python
def create_script(given_synopsys,isNewStory,lastConversation):
    if isNewStory:
        mainPrompt = f"""Think of a unique, stimulating,provocative,violent , ferocious, dopamine-boosting, crazy 5 minute TV drama story that fits the concept below and write a character's dialog or monologue.
       
user situation :
       
        """
        lastConversation = ""
    else:
        mainPrompt = f"""Think of a unique, stimulating,provocative,violent , ferocious, dopamine-boosting, crazy 5 minute TV drama story that follows on from the dialog below and write a character's line or monologue.


{lastConversation}




             
        """
    response = client.chat.completions.create(
    model="gpt-4o-mini",
        messages=[
            {"role": "system", "content": f"""You are a talented teenage drama story writer. The drama is about the school life of Korean high school students. Based on the given synopsis, write the Korean dialog for the characters.


Use the character information given below as a guide.


Characters


{prompt_persona}


{newCharactersPersona}


             """},
            {"role": "user", "content": f"""
{given_synopsys}
"""}
        ],
        functions=[function_description],
        function_call={"name": "format_conversation"},
        max_tokens=3000,
        temperature=1.1,
        top_p=0.8
    )


    return response.choices[0].message.function_call.arguments
```
