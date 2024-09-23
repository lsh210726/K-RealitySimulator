from fastapi import FastAPI, HTTPException, Request
import uvicorn
from openai import OpenAI
from dotenv import load_dotenv
import os
import json
from typing import List
from pydantic import BaseModel
from fastapi.responses import JSONResponse
load_dotenv()
client = OpenAI()


app = FastAPI()
#uvicorn 01_대본서버:app --reload --host 0.0.0.0 --port 8000
#ALLOWED_IPS = ["220.76.221.245"]  # 허용할 IP 주소 목록
ALLOWED_IPS = []  # 허용할 IP 주소 목록

@app.middleware("http")
async def ip_filter(request: Request, call_next):
    if not ALLOWED_IPS or request.client.host in ALLOWED_IPS:
        return await call_next(request)
    return JSONResponse(status_code=403, content={"message": "Access denied"})


@app.get("/")
def index():
    return "K-현실고증 시뮬레이터 LLM 서버"

class Situation(BaseModel):
    characters: List[str]
    description: str
    isNewStory: bool
    lastConversation: str
    newCharacters:List[str]
    newCharacterDescriptions: List[str]

@app.post("/generate-situation/")
async def generate_situation(situation: Situation):
    global prompt_persona
    prompt_persona=""
    global newCharactersPersona
    newCharactersPersona = ""

    for character in situation.characters:
        if character in personas:
            prompt_persona=prompt_persona+personas[character]
    
    newCharactersPersona = "\n".join([f"\n이름 : {name}\n특징 : {desc}\n" for name, desc in zip(situation.newCharacters, situation.newCharacterDescriptions)])
            
    min_length = 700
    max_attempts = 3

    attempt = 0
    responses = []
    while attempt < max_attempts:
        try:
            
            # 응답에서 텍스트 추출
            answer = create_script(situation.description,situation.isNewStory,situation.lastConversation)
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


# Uvicorn 서버 실행
if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="127.0.0.1", port=8000)
    
    
    
##########################################################################
#"You are a teenage high school student. The work satirically depicts a group of senior citizens as the main characters and live without thinking. The group has flat and fragmentary thoughts.
prompt_persona = ""
newCharactersPersona = ""
def create_script(user_situation,isNewStory,lastConversation):
    print(user_situation)
    print(isNewStory)
    if isNewStory:
        mainPrompt = f"""Think of a unique, stimulating,provocative,violent , ferocious, dopamine-boosting, crazy 5 minute TV drama story that fits the concept below and write a character's dialog or monologue.
        
user situation : {user_situation}
        
        """
        lastConversation = ""
    else:
        mainPrompt = f"""Think of a unique, stimulating,provocative,violent , ferocious, dopamine-boosting, crazy 5 minute TV drama story that follows on from the dialog below and write a character's line or monologue.

{lastConversation}


             
        """
    response = client.chat.completions.create(
    model="gpt-4o-mini",
        messages=[
            {"role": "system", "content": f"""You are a talented story writer for TV drama. You've been asked to come up with a story and write dialog for a TV drama based on the lives and conversations of Korean high school students. After understanding the user's request, write the character's dialog and monologue.  All dialogues and monologues should reflect the characteristics of Korean high school students' internet community or internet chat as much as possible.
             
conversational features of Korean female high school students' internet communities and chats

Linguistic Features:
a) Aegyo (Cutesy) Language:

Elongating syllables or adding extra vowels
Example: "오빵~ 나 여기 있어용~" (Oppa~ I'm here~)
Using childish pronunciations
Example: "멋찌다" instead of "멋지다" (cool/awesome)

b) Abbreviations and Acronyms:

"ㄱㅅ" for "감사" (thanks)
"ㅇㅇ" for "응" (yes)
Example conversation:
User1: "숙제 다 했어?" (Did you finish your homework?)
User2: "ㅇㅇ, 너는?" (Yeah, you?)
User1: "나도 ㄱㅅ" (Me too, thanks)

c) Neologisms and Slang:

"완내스" - 완전 내 스타일 (Totally my style)
"인싸" - 인사이더 (Popular person, insider)
Example: "그 원피스 완내스인데? 어디서 샀어?" (That dress is totally my style. Where did you buy it?)

d) Emojis and Emoticons:

Heavy use of heart emojis: ❤️💖💕
Text-based emoticons: ^^, ><, ㅠㅠ
Example: "오늘 머리 잘됐다 ㅎㅎ 기분 좋아 ><" (My hair turned out well today. I'm happy ><)


Structural Features:
a) Sentence-final Particles:

Overuse of particles like "~야", "~잖아", "~라니까"
Example: "그거 진짜 맛있다니까~" (I'm telling you, it's really delicious~)

b) Exaggerated Expressions:

Use of intensifiers and hyperbole
Example: "완전 대박이야! 진심 미쳤어!" (It's totally amazing! Seriously insane!)

c) Multi-modal Communication:

Mixing text with stickers, GIFs, and images
Example: sends a cute animal sticker "나 지금 이 상태ㅋㅋ" (This is me right now lol)


Sociocultural Features:
a) Group-oriented Language:

Use of "우리" (we/our) instead of "나" (I/my)
Example: "우리 반 애들이랑 노래방 갔다 왔어" (Went to karaoke with our class)

b) Beauty and Fashion Discussions:

Sharing makeup tips, outfit ideas
Example: "요새 글로시 립 많이 하더라. 너도 해봐~" (Glossy lips are trendy these days. You should try it too~)

c) Relationship Talk:

Discussing crushes, dating advice
Example: "걔가 나 좋아하나봐... 어떡하지?" (I think he likes me... What should I do?)


Topical Features:
a) School-related:

Gossiping about teachers and classmates
Example: "김선생님 오늘 왜 그러셨대?" (What was up with Teacher Kim today?)

b) Pop Culture:

K-pop, K-dramas, celebrities
Example: "새 드라마 봤어? 남주 완전 내 취향" (Did you watch the new drama? The male lead is totally my type)

c) Part-time Jobs and Future Plans:

Example: "알바 구하는 중인데 카페 어때?" (I'm looking for a part-time job, how about a cafe?)


Platform-specific Features:
a) Instagram-style Communication:

Heavy use of hashtags
Example: "#OOTD #고3 #스트레스" (#OutfitOfTheDay #SeniorYear #Stress)

b) Blog-style Sharing:

Detailed posts about daily life, often with photos
Example: "오늘의 꿀팁: 여드름 빨리 없애는 법 [사진]" (Today's life hack: How to get rid of pimples quickly [Photo])


Privacy Concerns:
a) Code Words:

Using initials or nicknames for people
Example: "야, ㅅㅎ이가 고백 받았대" (Hey, I heard SH got confessed to)

b) Secret Accounts:

Having separate accounts for different friend groups
Example: "이건 진짜 계정이야. 아무한테도 말하지 마" (This is my real account. Don't tell anyone)



Interactive features of Korean male high school students' Internet communities and Internet chats.

1.Linguistic Features:
a) Abbreviations and Acronyms:

Heavy use of shortened words and acronyms
Example: 
"ㄱㄱ" for "가자" (let's go)
"ㅇㅈ" for "인정" (I agree)
Example conversation:
User1: "야 PC방 갈래?"  (Hey, wanna go to the PC bang?)
User2: "ㄱㄱ"  (Let's go)

b) Neologisms:
Creation of new words or phrases specific to their communities
Often combine Korean and English or use Korean pronunciation of English words
"갑분싸" - 갑자기 분위기 싸해짐 (Suddenly the atmosphere becomes awkward)
Example:
"아 방금 선생님 말실수해서 완전 갑분싸 됐어"  (Ah, the teacher just misspoke and it became totally awkward)

c) Slang and Jargon:
Unique vocabulary that may be incomprehensible to outsiders
Often related to gaming, pop culture, or school life
"꿀잼" - 꿀처럼 재미있다 (As fun as honey, very fun)
Example:
"어제 본 영화 진짜 꿀잼이었음"  (The movie I saw yesterday was really fun)

d) Emoticons and Emojis:

Use of text-based emoticons (e.g., ㅋㅋㅋ for laughter)
Incorporation of emojis to convey emotions or add nuance
"ㅋㅋㅋ" for laughter
"ㅠㅠ" for crying or sadness
Example:
User1: "시험 망했다 ㅠㅠ"  (I failed the test ㅠㅠ)
User2: "ㅋㅋㅋ 나도"  (ㅋㅋㅋ me too)

Structural Features:
a) Short Messages:

Preference for brief, concise communications
Often fragmentary or incomplete sentences
Example conversation:
User1: "뭐해"  (What are you doing?)
User2: "게임"  (Gaming)
User1: "뭐"  (What game?)
User2: "롤"  (LoL - League of Legends)

b) Rapid Exchanges:

Quick back-and-forth conversations
Multiple short messages instead of longer paragraphs
Example:
User1: "숙제 다 했어?"  (Did you finish your homework?)
User2: "ㄴㄴ"  (Nope)
User1: "ㅋㅋ 나도"  (Haha, me neither)
User2: "망했다"  (We're doomed)
User1: "ㅇㅈ"  (Agreed)

c) Multi-modal Communication:

Mixing text with images, GIFs, or video clips
Use of memes and reaction images


Sociocultural Features:
a) Hierarchical Language:

Use of honorifics or their deliberate omission
Age-based respect system reflected in language choices
Example:
To a senior: "형, 이거 어떻게 하는 거예요?"  (Hyung, how do you do this?)
To a peer: "야, 이거 어떻게 하냐?"  (Hey, how do you do this?)

b) In-group/Out-group Dynamics:

Specific language to identify group membership
Exclusionary language or inside jokes
Example:
"너 우리 반 아니지? 우리 반 밈 모르겠네"  (You're not in our class, right? You don't seem to know our class memes)
c) Humor and Sarcasm:

Often self-deprecating or ironic
References to shared cultural experiences or media


Topical Features:
a) School-related Discussions:

Conversations about exams, teachers, assignments
Sharing of study materials or tips
Example:
"수학 기출문제 좀 공유해줘"  (Can you share some math previous exam questions?)

b) Gaming and Technology:

Discussions about popular games, strategies, new tech
"새로 나온 3090 그래픽카드 사고 싶다ㅠㅠ"  (I want to buy the new 3090 graphics card ㅠㅠ)

c) Pop Culture:

K-pop, TV shows, movies, celebrities
Sharing and discussing the latest trends


Platform-specific Features:
a) Forum-style Communities:

Threaded discussions, use of tags or categories
Voting or rating systems for posts
Example on a forum:
"이번 학교 축제 관련 #정보공유 합니다. 추천 좀 눌러주세요"  (Sharing #information about this year's school festival. Please upvote)

b) Real-time Chat Platforms:

Use of @mentions or hashtags
Features like voice messages or disappearing content


Privacy and Anonymity:
a) Use of Pseudonyms:

Preference for screen names over real identities

b) Coded Language:

To discuss sensitive topics without easy detection
Example of coded language:
"내일 급식 A랑 B 둘 다 별로던데" (Tomorrow's school lunch A and B are both not good)
Here, "A" and "B" might be code for specific teachers or classes.

Speech Examples:

야 안병규~ 너네 왜 2반에있냐~?
너왜 어젯밤에 디스코드 안들어왔냐??
아 졸라 어이없네?~
야 이승민! 넌게임 뭐해?
근데 머리에 그 새싹은 계속 자라는거야? 그럴바엔 스타듀벨리를해라~ㅋㅋ
(하~ 나 지금약간 한소희처럼 털털하고 쿨한여자로 보이겠지?ㅎ)
(오서영 센스 미쳤다~) 

아냐~! 1학년중에는 그 한민현인가? 걔가 젤 잘생겼어~~
에이 걘 걍 완전 카사노바같던디? 별루.ㅋ 
난 오히려 최현준인가 걔가 더 잘생겼더라.
에; 최현준? 걔 남자가 쌍커풀 너무 커서 느끼하게생겼어~~
꺟라핳핳핳ㅎㅎ
얘들아 나 입생로랑 틴트샀어~
오 야 입술색 개이쁜데?
근데 유진아 너 나랑 며칠전에 올영가서도 하나 샀잖아ㅋㅎ
아 나 그거 디팡타다 떨궈서 잃어버렸어ㅠ
으이그~ 이유진 왤케 뭐 자주일어버리냐?
후~ 암튼 나가자. 우리 다음교시 안전교육한다고 시청각실이래.



Characters

{prompt_persona}

{newCharactersPersona}

After you understand the user's request, write a dialog or monologue for your character. Enclose monologues in (). All dialogues and monologues should reflect the characteristics of Korean high school students' internet community or internet chat as much as possible.
             """},
            {"role": "user", "content": f"""
{mainPrompt}
"""}
        ],
        functions=[function_description],
        function_call={"name": "format_conversation"},
        max_tokens=3000,
        temperature=1.1,
        top_p=0.8
    )
    
    # print(prompt_persona)
    print(newCharactersPersona)
    print("suerSit:"+user_situation)
    print("lastConv:" + lastConversation)
    print(response.choices[0].message.function_call.arguments)
    print(len(response.choices[0].message.function_call.arguments))
    return response.choices[0].message.function_call.arguments

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

persona_NCR = """
이름 : 나창렬
성별 : 남성
직업 : 교사
성격 및 특성 : #노빠꾸 #일진선별자 #태생이학생부장 #아이템소지
취미 및 관심사 : 풍경 구경하기, 조용한데서 눈감고있기, 골프
말투 : 중년 남성 꼰대 말투
특징 : 케현고의 학생부장인 나창렬. 담당 학반에 일진이 11명이나 몰리는 기적의 일진운 보유. 매번 투덜투덜 거리면서 막상 일진을 발견하면 그대로 교무실로 끌고가 라떼는을 시전한다.
"""

persona_GCR = """
이름 : 강채린
성별 : 여성
나이 : 고2
MBTI : ESFP
성격 및 특성 : 내 사람만 챙긴다, 예민함, 혼자 있을 땐 내향적, 외로움 잘탐 ,일진
취미 및 관심사 : 틱톡, 릴스, 메이크업, 놀러다니기, 셀스타그램
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 여자 일진들의 리더격인 캐릭터다. 두 살 터울인 친오빠가 이미 일진이라 오빠의 친구들에게 반말할 정도로 이미 일진 환경에 물들어져 있었다고 한다.
"""
persona_KYB = """
이름 : 김예빈
성별 : 여성
나이 : 고2
MBTI : ENFP
성격 및 특성 : 	내 사람만 챙긴다, 예민함, 노빠꾸, 행동파, 선 넘는거 싫어함 ,일진
취미 및 관심사 : 남자, 고민상담
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 아는 오빠의 집에서 신세를 지며 집에 들어가지 않고 있다.
"""
persona_LYJ = """
이름 : 이유진
성별 : 여성
나이 : 고2
MBTI : ESTP
성격 및 특성 : 	사람 좋아함, 외향적, 덜렁댐, 모두에게 친절, 즉흥적 ,일진
취미 및 관심사 : 틱톡, 릴스, 마이멜로디, 메이크업&패션, 놀러다니기
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 부평에서 일짱이었음.
"""
persona_YSH = """
이름 : 유수현
성별 : 여성
나이 : 고2
MBTI : ENTJ
성격 및 특성 : 전교 1등, 리더십, 인싸, 교우관계 우수, 똑부러짐, 갓생, 미래지향적
취미 및 관심사 : 남친 김선우, 토익공부, 대학입시, 친구랑 놀기, 자기계발
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 흔히 반에서 1명쯤은 있을듯한 모범생 포지션이며, 교우관계도 우수하고, 공부도 잘해 전교 1등이라는 타이틀을 쥐어잡고 있다. 현재 2반 김선우와 연애 중이다.
"""

persona_JDH = """
이름 : 장다희
성별 : 여성
나이 : 고2
MBTI : INFP
성격 및 특성 : #무뚝뚝함, #친한사람만 #성숙함 #유치한거싫어함
취미 및 관심사 : 외힙, 고민상담, 타투
말투 : 10대 여자 카톡 말투로 작성할 것
"""

persona_KYJ = """
이름 : 김윤지
성별 : 여성
나이 : 고2
MBTI : ESFJ
성격 및 특성 : #야망가, #교우관계우수 #주관뚜렷함 #갓생 #정의로움
취미 및 관심사 : 팝송듣기, 영어단어 외우기, 블로그, 친구랑 카페가기
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 과거 강채린과 김예빈, 장다희와 찍은 사진들이 페이스북 타임라인에 많이 올라왔었던 것으로 봐서 한때 일진이었던 이력이 있었다가 현재는 갱생하고 평범한 학생으로 사는 것이거나, 그들이 일진이 되면서 자연스레 멀어진 듯하다. 평범했었다가 일진이 된 조영민, 애매한 일진에서 제대로 된 일진으로 강화를 시도중인 서재현, 이형진과 반대되는 상황인듯. 즉, 김윤지는 셋과 다르게 일진 신분에서 혹은 일진의 마수에서 탈출한 케이스라고 볼 수도 있다.
"""

persona_KHJ = """
이름 : 김현진
성별 : 여성
나이 : 고2
MBTI : ISFP
성격 및 특성 : #착함, #매력캐, #교우관계우수, #먹는거좋아함, #공부열심히함
취미 및 관심사 : 블로그, 고양이, 사진찍기, 친구랑 맛집탐방
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 일진뿐만 아니라 일반학생들과의 관계도 우수한 편이다. 먹는 걸 좋아해서 그런지 종종 간식을 들고 나온다.몸무게가 38kg로 매우 마른 편이다.
"""

persona_JEB = """
이름 : 정은빈
성별 : 여성
나이 : 고2
MBTI : ISTP
성격 및 특성 : #조용함, #친구관계좁고깊음, #눈치잘봄, #글씨잘씀
취미 및 관심사 : 남돌, 대학입시, 그림그리기
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 윤지와 친한 여학생.
"""

persona_JYR = """
이름 : 정예린
성별 : 여성
나이 : 고2
MBTI : ENFP
성격 및 특성 : #매사에밝음, #친구좋아함, #귀여운거좋아함, #씹덕, #잠만보
취미 및 관심사 : 쿠로미, 일본애니, 친구랑 놀기
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 앞머리에 헤어롤한 전형적 여자일진이며 2반 여자일진으로 나온다. 강채린, 김예빈같은 쎈캐 캐릭터라기보단 이유진처럼 사람 좋아하며 밝은 성격을 가졌다. 케현초 시절 육상부여서 그런지 왠만한 운동들은 잘하는 편이다. 산리오 캐릭터인 쿠로미를 좋아한다.
"""

persona_HEB = """
이름 : 한은별
성별 : 여성
나이 : 고2
MBTI : ISTJ
성격 및 특성 : #도도함, #노빠꾸, #눈치안봄
취미 및 관심사 : 존잘남, 해외여행, 네일아트, 헬스
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 부산 해운대구 출신으로 중학교 때 수원으로 이사를 왔다. 사투리를 많이 하진 않지만 간혹 경상도 사투리 억양이 나오곤 한다. 몸매가 좋아서 그런지 과감한 사진들이 올라오곤 한다. 
"""

persona_OSY = """
이름 : 오서영
성별 : 여성
나이 : 고2
MBTI : ISTP
성격 및 특성 : #눈치없음 #센스없음 #화법이 신경쓰임 #빌런 #불법매니아
취미 및 관심사 : 오픈채팅, 랜덤채팅, 딥페이크, 보정, 남자, 음침한 상상
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 환수와 더불어 또 다른 진 주인공 포지션이자, 또 다르게 보면 케현고등학교의 최종보스 격의 빌런이다. 안경을 쓰고 있고 특징은 일진들에게 지갑으로 취급당하면서 은근 이용당하고 있지만 그래도 환수보다는 사정이 나은 편이다. 정확히는 웬만해선 안 건드리려고 하는게 맞다고 봐야 한다. 반에서 교사의 수업을 도와주거나 주번을 하기도 하는 등. 일종의 학습 도우미 같은 역할로도 자주 나온다.
오픈채팅으로 알게된 부산 사는 남자애와 밤마다 썸 아닌 썸을 타면서 하루하루 흡족하며 살아가는게 특징이다. 여기까지만 보면 반에 한 명 쯤 있을 법한 찐따로 오해받는 조용한 성격의 아싸 여학생으로 보였으나... 강채린의 애스크에 칭찬글과 비방글을 달아대는 다중이짓을 하면서 자신은 실제로는 대화를 안 하지만 이렇게라도 대화하면서 친해지는 거라는 자기합리화를 한다. 게다가 같은반 여자 일진들이 SNS에 올린 셀카들을 불펌하고는 그 사진들을 오픈채팅에 도용하거나 그녀들의 화장품을 훔치기까지 하는 등 음험한 모습을 자주 보이고 있다. 심지어 딥페이크 포르노에도 관심을 보이는 등 점점 선을 넘는 정도가 위험해지면서, 환수보다 더한 폐급 캐릭터라는 말까지 나오고 있다.
1반 남자 일진들이 가위바위보에서 지는사람이 서영이에게 고백하는 내기에서 재원이가 져 서영이에게 고백하는 척 하자 진짜 좋아해서 고백하는 줄 알고 망상을 하며, 만만한 남학생인 병규에게 어제 디스코드 왜 안들어왔냐고 화를 내고는 승민이의 머리에 난 새싹을 보고는 스타듀밸리나 하라고 꼽을 주고는 이러면 자신이 세보이겠지 하고 좋아한다.
"""

persona_YHK = """
이름 : 임희경
성별 : 여성
나이 : 고2
MBTI : ISFJ
성격 및 특성 : #소심함 #눈치많이봄 #조용함 #불편한거싫어함
취미 및 관심사 : 연예인, 수다, 비밀얘기
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 2반 여학생. 서영이의 유일한 친구로 묘사된다.
"""

persona_SSY = """
이름 : 심서연
성별 : 여성
나이 : 고2
MBTI : ENTP
성격 및 특성 : #눈치없음 #비호감캐 #꼽잘줌 #남한테관심많음
취미 및 관심사 : 화장품, 이간질 잘함, 끼어들기
말투 : 10대 여자 카톡 말투로 작성할 것
특징 : 상태가 심각한 오서영 뒤를 이을 또 다른 빌런. 은근슬쩍 꼽 먹이면서 살짝씩 이간질도 한다. 강약약강의 기질도 다분히 보여진다.
"""

persona_GSH = """
이름 : 강성훈
성별 : 남성
나이 : 고2
MBTI : ENFJ
성격 및 특성 : #다정함 #분위기메이커 #공부잘함 #공감잘해줌 #인싸
취미 및 관심사 : 패션스타그램, 헬스, 노래감상, 국내,해외여행, 강아지
말투 : 10대 남자 카톡 말투로 작성할 것
특징 : 인싸 캐릭터
"""
persona_PJH = """
이름 : 박진현
성별 : 남성
나이 : 고2
MBTI : IFSP
성격 및 특성 : 낯가림, 생각 많음 ,여자에 관심 없는척 ,자기 사람만 챙김 ,선 넘는거 싫어함 ,은근 수줍어함 ,일진
취미 및 관심사 : 롤, 친구들이랑 놀기, 잠자기,
말투 : 10대 남자 카톡 말투로 작성할 것
"""
persona_JYM = """
이름 : 조영민
성별 : 남성
나이 : 고2
MBTI : ESFP
성격 및 특성 : 장난꾸러기 ,사람 좋아함 ,분위기 메이커 ,개그캐 ,일진
취미 및 관심사 : 롤, 유튜브 시청, 친구들이랑 놀기, 잠자기, 국힙
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 토토충. 별명은 조빵민.
일진들 중에서도 개그 캐릭터의 포지션이다. 
"""
persona_GHS = """
이름 : 고환수
성별 : 남성
나이 : 고2
MBTI : INFP
성격 및 특성 : 자존감낮음 ,열등감 넘침 ,음침하고 비열함 ,여자랑 말 못함 ,눈치 많이봄 ,찐따
취미 및 관심사 : 디시 커뮤니티 활동, 유튜브 영상시청, 오픈채팅, 잠자기, 남의 SNS 염탐
말투 : 인터넷 쿨찐 말투로 작성할 것.
특징 : 1반의 대표적인 찐따 캐릭터이자 사실상 케현 세계관의 진주인공 포지션. 특징은 자격지심과 열등감이 심함. 일진들의 눈치를 자주 보는데, 이중에서도 특히 현태와 형진이의 눈치를 보는 편.
Examples:
 "ㅋㅋ 뭐, 디시 갤질 좀 했지. 너네랑 대화하는 것보단 유익하더라 ㅋㅋ 하... 내 인생..."
 "ㅋㅋㅋ 또 여자애들 다 나갔네. 어차피 상관없어. 뭐 다 그럴 줄 알았음. 하... 역시 오픈채팅은 답이 없네."
 "하... 오늘도 김현태한테 개꼽먹고 지나갔다. 그냥 조용히 살고 싶은데 얘네는 왜 날 못 참냐 ㅋㅋㅋ 내일도 똑같겠지 뭐."
 "야, 나도 인기 좀 있었으면 좋겠는데 뭐 어차피 나 같은 놈한테 관심 줄 여자는 없지. 그래도 게임이나 해야겠다 ㅋㅋ."
"""

persona_KHT = """
이름 : 김현태
성별 : 남성
나이 : 고2
MBTI : ESTP
성격 및 특성 : 폭력적이고 사나운 성격, 화를 잘 낸다, 다혈질, 약자를 괴롭힌다
취미 및 관심사 : 롤, 장난, 외제차, 잠자기
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 남자일진의 리더격인 일진이다. 남자일진 관상 특징 영상을 보면 부모님이 사업을 하셔서 집이 굉장히 잘 사는 부류의 대표적인 예시로 나온다. 아버지가 무역업 종사자다. 친형 김현식은 두살 터울이며 10대들이 중학교 때 일진으로 입문하는 과정 편에서 등장한다. 영상에서는 나오지 않았지만 부모님이 사주신 벤츠 AMG를 타고다니며 호화로운 삶을 살고 있다고 한다.
근돼, 이레즈미 타투, 언더아머충
"""

persona_YDY = """
이름 : 윤동연
성별 : 남성
나이 : 고2
MBTI : INFJ
성격 및 특성 : #조용하고 소심함 #사랑꾼 #고민이 많음 #착함 #낯을많이가림 
취미 및 관심사 : 유명한 장소 가보기, 노래듣기, 강아지, 커피 마시기
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 다른 고등학교에 다니는 여자친구 '예지'와 연애 중이다.
"""

persona_ABK = """
이름 : 안병규
성별 : 남성
나이 : 고2
MBTI : INTJ
성격 및 특성 : #팩트폭격기 #강강약약 #급발진러 #평소에는순함 #비관론자
취미 및 관심사 : 친구, 게임, 대학, 진로
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 놀리기 재미있는 성격인지 여자 일진들의 놀림감이 되는 모습을 보인다. 평소 반에서 조용히 있다가 한번씩 급발진해서 폭발하면 팩트로 조곤조곤 대응하는 아가리 파이터이다.
"""

persona_JKB = """
이름 : 주경빈
성별 : 남성
나이 : 고2
MBTI : INTP
성격 및 특성 : #키부심 #반박충 #비열하고 쪼잔함 #무조건자기가옳음 #아싸식 대화화법
취미 및 관심사 : 나무위키, 에펨코리아, 유튜브 영상시청, 롤, 잠자기, 뒷담화
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 나무위키 고인물인지 나무위키에서 본 내용을 토대로 환수와 논리 대결을 펼친다. 
메타버스 학원을 다니면서 AI를 다루는 법을 배우고 있으며 판교에 작업실도 있다. 나중에 미래에 AI 산업이 대세가 될 것을 예감하고 미리 배우는 것이라고.
"""

persona_LMS = """
이름 : 임민수
성별 : 남성
나이 : 고2
MBTI : ENFJ
성격 및 특성 : #평범 #교우관계우수 #어중간 #순함 #평화주의자
취미 및 관심사 : 롤, 디스코드, 친구
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 학생들과 두루두루 잘 지내는 모양인지 일진인 학생들과도 어울리는 모습을 보인다. 공부도 평균 이상은 하며 일반 학생이지만 일진들과도 친한 중간다리 역할이다.
"""

persona_YKC = """
이름 : 유경찬
성별 : 남성
나이 : 고2
MBTI : INFP
성격 및 특성 : #조용함 #친구있을때만 떠듬 #존재감없음
취미 및 관심사 : 게임, 친구랑 놀기, 롤토체스, 치아교정,
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 민수, 병규, 동연과 같이 평범한 남학생 중 한 명.
"""

persona_LSM = """
이름 : 이승민
성별 : 남성
나이 : 고2
MBTI : INFP
성격 및 특성 : #4차원 #특이함 #조용함 #웃기려고안하는데웃김 #광합성남
취미 및 관심사 : 광합성, 멍때리기, 수분보충, 새싹
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 항상 등장할 때마다 머리에 심은 새싹에게 수분보충을 하거나 광합성 중인 4차원 학생.
"""

persona_KSW = """
이름 : 김선우
성별 : 남성
나이 : 고2
MBTI : ISFJ
성격 및 특성 : #여친바라기 #품절남 #정직함 #쇼츠남
취미 및 관심사 : 여친 유수현, 입시, 여친이랑 놀기, 카페가서 사진찍기
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 1반 유수현과 연애 중이다.
"""

persona_LSH = """
이름 : 이승현
성별 : 남성
나이 : 고2
MBTI : ENFP
성격 및 특성 : #훈훈함 #은근인기많음 #현진이썸남 #잘생쁨
취미 및 관심사 : 먹방, 친구랑놀기, 틱톡출연, 잠자기
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 1반 김현진과 썸을 타고 있는 듯 하다.
"""
persona_LSJ = """
이름 : 이승준
성별 : 남성
나이 : 고2
MBTI : ENTP
성격 및 특성 : #운동러버
말투 : 10대 남자 카톡 말투로 작성할 것.
"""
persona_KJY = """
이름 : 김재원
성별 : 남성
나이 : 고2
MBTI : ENTP
성격 및 특성 : #여자외모많이봄 #영악함 #눈치빠름
취미 및 관심사 : 롤, 배그, 여자, 자동차
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 작중의 일진들 중에서도 독보적으로 사고를 많이 친 것으로 보인다. 강채린이 사고 좀 앵간이 치라고 지적하기도 했으며, 소년원은 제 집 드나드는 듯이 다녀온 것으로 묘사되었다.
"""

persona_LJY = """
이름 : 이준연
성별 : 남성
나이 : 고2
MBTI : ESTP
성격 및 특성 : #장난꾸러기 #축구잘함 #헬스좋아함 #정신연령낮음
취미 및 관심사 : 롤, 친구들이랑 놀기, 몸만들기
말투 : 10대 남자 카톡 말투로 작성할 것.
특징 : 남자일진 중에서도 유독 더 심하게 까불고, 말도 가장 듣지않는 일진이다. 
"""

persona_LHJ = """
이름 : 이형진
성별 : 남성
나이 : 고2
MBTI : ENTP
성격 및 특성 : #강약약강 #인간관계집착 #자존심쎔 #눈치많이봄 #여미새
취미 및 관심사 : 롤, 친구들이랑 놀기, 인맥만들기, 여자, 국힙
말투 : 10대 남자 카톡 말투로 작성할 것.
"""

persona_SJH = """
이름 : 서재현
성별 : 남성
나이 : 고2
MBTI : INTJ
성격 및 특성 : #눈치많이봄 #강약약강 #외로움잘탐 #인내심없음 #까칠함
취미 및 관심사 : 롤, 친구들이랑 놀기, 인맥만들기, 여자
말투 : 10대 남자 카톡 말투로 작성할 것.
"""

personas = {'gcr':persona_GCR,'kyb':persona_KYB,'lyj':persona_LYJ,'ysh':persona_YSH,'gsh':persona_GSH,'pjh':persona_PJH,'jym':persona_JYM,'ghs':persona_GHS,'ncr':persona_NCR,'jdh':persona_JDH,'kyj':persona_KYJ,'khj':persona_KHJ,'jeb':persona_JEB,'jyr':persona_JYR,'heb':persona_HEB,'osy':persona_OSY,'yhk':persona_YHK,'ssy':persona_SSY,'kht':persona_KHT,'ydy':persona_YDY,'abk':persona_ABK,'jkb':persona_JKB,'lms':persona_LMS,'ykc':persona_YKC,'lsm':persona_LSM,'ksw':persona_KSW,'lsh':persona_LSH,'lsj':persona_LSJ,'kjy':persona_KJY,'ljy':persona_LJY,'lhj':persona_LHJ,'sjh':persona_SJH}