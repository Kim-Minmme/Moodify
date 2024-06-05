from Expression_Measurement_API.emotion import (
    init_hume_client,
    extract_emotions,
    emotion_to_color,
    logging
)

from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI()
hume_client = None
config = None

class Item(BaseModel):
    image: str

@app.post("/create/color")
async def create_color(item: Item):
    global hume_client, config
    if hume_client is None or config is None:
        hume_client, config = init_hume_client()
    emotions = await extract_emotions(hume_client, config, item.image)
    color = emotion_to_color(emotions)
    logging.info(f"Calculated RGB: {color}")
    return color

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=20090)