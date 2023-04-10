import requests
import yaml

PROJECT_ID = "newagent-jfey"
SESSION_ID = "0"

url = "https://dialogflow.googleapis.com/v2/projects/{}/agent/sessions/{}:detectIntent".format(PROJECT_ID, SESSION_ID)

def get_intent(text):

    request_json_body = {
        "query_input": {
            "text": {
                "text": text,
                "language_code": "en-US"
            }
        }
    }

    response = requests.post(url, headers=headers, json=request_json_body)
    if response.status_code == 200:
        data = response.json()['queryResult']
        print("Intent:", data['intent']['displayName'])
        print("Confidence:", data['intentDetectionConfidence'])
    else:
        print(response.status_code)
        print(response.json())



if __name__ == "__main__":

    try: 
        creds = yaml.load(open("creds.yaml"), Loader=yaml.Loader)
        API_KEY = creds["api-key"]

        headers = {
        # "Content-Type:": "application/json", #;+charset=utf-8",
            "x-goog-user-project": PROJECT_ID,
            "Authorization": "Bearer {}".format(API_KEY),
            "Accept": "application/json"

        }

        texts = [
            "Pause the buzzer",
            "Stop the buzzer please",
            "I'm leaving for the day",
            "I'm back home!",
            "Going to sleep now",
            "Good morning, I'm up!"
        ]
        
        for text in texts: 
            get_intent(text)

    except Exception as e:
        print(e)
        print("API key not set up. Please create a file called creds.yaml which contains the API key.")

    