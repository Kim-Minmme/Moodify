# from fastapi import FastAPI
# from starlette.requests import Request

# app = FastAPI()

# @app.post("/test")
# async def test(request: Request):
#     body = await request.json()
#     print(body)
#     return {"message": "Request received"}

# if __name__ == "__main__":
#     import uvicorn
#     uvicorn.run(app, host="0.0.0.0", port=8000)

from fastapi import FastAPI, Request
from pydantic import BaseModel
import base64
from io import BytesIO
from PIL import Image, ImageTk
import tkinter as tk
from tkinter import Label

app = FastAPI()

class ImageData(BaseModel):
    picture_base64: str

root = tk.Tk()
root.title("Image Display")
label = Label(root)
label.pack()

@app.post("/test")
async def decode_image(data: ImageData):
    image_data = base64.b64decode(data.picture_base64)
    image = Image.open(BytesIO(image_data))
    display_image(image)
    return {"message": "Image received and displayed"}

def display_image(image: Image.Image):
    # Convert PIL image to ImageTk.PhotoImage
    image_tk = ImageTk.PhotoImage(image)
    
    label.config(image=image_tk)
    label.image = image_tk  # Keep a reference to avoid garbage collection

if __name__ == "__main__":
    import threading
    import uvicorn

    # Start the FastAPI server in a separate thread
    def run_server():
        uvicorn.run(app, host="0.0.0.0", port=8000)

    server_thread = threading.Thread(target=run_server)
    server_thread.start()

    # Start the Tkinter main loop
    root.mainloop()


