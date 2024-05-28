import os
import logging

from hume import HumeStreamClient
from hume.models.config import FaceConfig

from fastapi import FastAPI
from pydantic import BaseModel

hume_client = None
hume_socket = None

COLOR_MAPPING = {
    'Admiration': (255, 215, 0),
    'Adoration': (255, 192, 203),
    'Aesthetic Appreciation': (230, 230, 250),
    'Amusement': (255, 165, 0),
    'Anger': (255, 0, 0),
    'Anxiety': (173, 216, 230),
    'Awe': (128, 0, 128),
    'Awkwardness': (245, 245, 220),
    'Boredom': (128, 128, 128),
    'Calmness': (152, 251, 152),
    'Concentration': (0, 0, 255),
    'Confusion': (165, 42, 42),
    'Contemplation': (0, 128, 128),
    'Contentment': (144, 238, 144),
    'Craving': (139, 0, 0),
    'Desire': (255, 105, 180),
    'Determination': (0, 0, 139),
    'Disappointment': (169, 169, 169),
    'Disgust': (128, 128, 0),
    'Distress': (139, 0, 139),
    'Doubt': (255, 255, 224),
    'Embarrassment': (240, 128, 128),
    'Empathic Pain': (128, 0, 0),
    'Entrancement': (0, 255, 255),
    'Envy': (50, 205, 50),
    'Excitement': (255, 255, 0),
    'Fear': (72, 61, 139),
    'Guilt': (152, 251, 152),
    'Horror': (0, 0, 0),
    'Interest': (64, 224, 208),
    'Joy': (255, 255, 0),
    'Love': (255, 0, 0),
    'Nostalgia': (112, 66, 20),
    'Pain': (139, 0, 0),
    'Pride': (128, 0, 128),
    'Realization': (173, 216, 230),
    'Relief': (144, 238, 144),
    'Romance': (255, 192, 203),
    'Sadness': (0, 0, 255),
    'Satisfaction': (144, 238, 144),
    'Shame': (139, 0, 0),
    'Surprise (negative)': (255, 69, 0),
    'Surprise (positive)': (255, 215, 0),
    'Sympathy': (255, 255, 224),
    'Tiredness': (175, 238, 238),
    'Triumph': (255, 215, 0)
}

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

async def initialize_hume_client():
    global hume_client, hume_socket
    HUMEAI_API_KEY = os.getenv("HUMEAI_API_KEY")
    hume_client = HumeStreamClient(HUMEAI_API_KEY)
    config = FaceConfig(identify_faces=True)
    hume_socket = await hume_client.connect([config])

async def extract_emotions(image):
    global hume_socket
    try:
        result = await hume_socket.send_bytes(image)
        emotions = result["face"]["predictions"][0]['emotions']
        return emotions
    except Exception as e:
        logging.error(f"An error occurred: {e}")
        return []

def int_to_hex(r, g, b):
    return f'#{r:02x}{g:02x}{b:02x}'

def emotion_to_color(emotions):
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

    return int_to_hex(r, g, b)

app = FastAPI()

class Item(BaseModel):
    image: str

@app.post("/create/color/")
async def create_color(item: Item):
    image = item.image
    emotions = await extract_emotions(image)
    color = emotion_to_color(emotions)
    logging.info(f"Calculated RGB: {color}")
    return color

async def main():
    await initialize_hume_client()
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=25565)

if __name__ == "__main__":
    main()