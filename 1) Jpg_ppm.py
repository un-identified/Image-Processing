import cv2
import tkinter as tk
from tkinter import filedialog

def jpg_to_ppm():
    root = tk.Tk()
    root.withdraw() 

    # select a JPG file
    input_file = filedialog.askopenfilename(title="Select JPG file", filetypes=(("JPG files", "*.jpg"), ("All files", "*.*")))
    
    if not input_file:
        print("No file selected. Exiting...")
        return
    
    # Read the image
    input_image = cv2.imread(input_file)
    
    if input_image is None:
        print("Error: Could not open or find the image:", input_file)
        return
    
    # Extract image dimensions
    height, width, _ = input_image.shape
    
    # Output file path
    output_file = "input.ppm"

    with open(output_file, 'wb') as output_file:
        
        output_file.write(bytearray("P6\n{} {}\n255\n".format(width, height), 'ascii'))
        
        # Convert and write image data to PPM format
        for y in range(height):
            for x in range(width):
                pixel = input_image[y, x]
                output_file.write(bytearray([pixel[2], pixel[1], pixel[0]]))

    print("Image converted successfully from JPG to PPM:", output_file.name)

jpg_to_ppm()
