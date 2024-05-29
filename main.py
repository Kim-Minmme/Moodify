from Expression_Measurement_API.emotion import *

from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI()
hume_socket = init_hume_client()

class Item(BaseModel):
    image: str

@app.post("/create/color")
async def create_color(item: Item):
    image = item.image
    emotions = await extract_emotions(hume_socket, image)
    color = emotion_to_color(emotions)
    logging.info(f"Calculated RGB: {color}")
    return {"color": color}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=25565)