import os
import asyncio
import logging
from pprint import pprint

from hume import HumeStreamClient
from hume.models.config import FaceConfig

with open('api.key', 'r') as file:
    HUMEAI_API_KEY = file.read().replace('\n', '')

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# Color mapping defined as a constant
COLOR_MAPPING = {
    'Admiration': (255, 204, 153),
    'Adoration': (255, 153, 204),
    'Aesthetic Appreciation': (153, 255, 204),
    'Amusement': (255, 255, 153),
    'Anger': (255, 0, 0),
    'Anxiety': (255, 128, 0),
    'Awe': (204, 153, 255),
    'Awkwardness': (255, 204, 102),
    'Boredom': (128, 128, 128),
    'Calmness': (102, 255, 178),
    'Concentration': (0, 102, 204),
    'Contemplation': (153, 102, 204),
    'Confusion': (255, 102, 102),
    'Contempt': (153, 0, 76),
    'Contentment': (204, 255, 102),
    'Craving': (255, 102, 255),
    'Determination': (102, 51, 153),
    'Disappointment': (102, 102, 255),
    'Disgust': (102, 255, 102),
    'Distress': (255, 102, 0),
    'Doubt': (204, 102, 255),
    'Ecstasy': (255, 0, 255),
    'Embarrassment': (255, 204, 153),
    'Empathic Pain': (255, 102, 51),
    'Entrancement': (102, 255, 255),
    'Envy': (0, 255, 0),
    'Excitement': (255, 255, 0),
    'Fear': (0, 0, 255),
    'Guilt': (102, 102, 153),
    'Horror': (51, 0, 102),
    'Interest': (255, 153, 51),
    'Joy': (255, 204, 0),
    'Love': (255, 0, 0),
    'Pride': (204, 102, 0),
    'Realization': (0, 204, 255),
    'Relief': (102, 255, 153),
    'Romance': (255, 51, 153),
    'Sadness': (0, 0, 102),
    'Satisfaction': (204, 255, 51),
    'Desire': (255, 0, 255),
    'Shame': (153, 0, 51),
    'Surprise (negative)': (204, 0, 204),
    'Surprise (positive)': (204, 204, 0),
    'Sympathy': (255, 102, 178),
    'Tiredness': (102, 102, 102),
    'Triumph': (255, 153, 204)
}

async def extract_emotions():
    # HUMEAI_API_KEY = os.getenv("HUMEAI_API_KEY")
    client = HumeStreamClient(HUMEAI_API_KEY)
    config = FaceConfig(identify_faces=True)
    
    async with client.connect([config]) as socket:
        while True:
            try:
                result = await socket.send_file("./example.jpg")
                emotions = result["face"]["predictions"][0]['emotions']
                # yield emotions
                return emotions
                await asyncio.sleep(1)  # Adjust the delay as needed
            except Exception as e:
                logging.error(f"An error occurred: {e}")

def emotion_to_rgb(emotions):
    r, g, b = 0, 0, 0
    total_score = 0

    for emotion in emotions:
        name, score = emotion['name'], emotion['score']
        if name in COLOR_MAPPING:
            r += COLOR_MAPPING[name][0] * score
            g += COLOR_MAPPING[name][1] * score
            b += COLOR_MAPPING[name][2] * score
            total_score += score

    if total_score > 0:
        r = int(r / total_score)
        g = int(g / total_score)
        b = int(b / total_score)

    return (r, g, b)


def top3_emotions(emotions):
    return sorted(emotions, key=lambda x: x['score'], reverse=True)[:3]

async def main():
    flag = 1

    if flag == 0:
        # 계속 도는 ver
        async for emotions in extract_emotions():
            rgb = emotion_to_rgb(emotions)
            logging.info(f"Calculated RGB: {rgb}")
            print("[ Emotions ]")
            pprint(top3_emotions(emotions))

    else:
        # 1번만 도는 ver
        rgb = emotion_to_rgb(extract_emotions())
        logging.info(f"Calculated RGB: {rgb}")
        print("[ Emotions ]")
        pprint(top3_emotions(emotions))

if __name__ == "__main__":
    asyncio.run(main())