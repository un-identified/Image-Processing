import cv2
import numpy as np

def ppm_to_jpg(input_file, output_file):
    # Read the input PPM image
    with open(input_file, 'rb') as ppm_file:
        header = ppm_file.readline().decode('utf-8').strip()
        if header != 'P6':
            print("Error: Unsupported PPM format")
            return
        while True:
            line = ppm_file.readline().decode('utf-8').strip()
            if not line.startswith('#'):
                break
        # Read width, height, and maximum color value
        width, height = map(int, line.split())
        max_val = int(ppm_file.readline().decode('utf-8').strip())

        # Read pixel data
        ppm_data = ppm_file.read()

    # Convert PPM data to NumPy array
    ppm_image = np.frombuffer(ppm_data, dtype=np.uint8)

    # Reshape the array to match image dimensions
    ppm_image = ppm_image.reshape((height, width, 3))

    # Write the NumPy array as a JPG image
    cv2.imwrite(output_file, cv2.cvtColor(ppm_image, cv2.COLOR_RGB2BGR))

    print("Image converted successfully from PPM to JPG:", output_file)

ppm_to_jpg("output.ppm", "output.jpg")
