import random
from PIL import Image, ImageDraw

def generate_starfield(width=512, height=512, star_density=0.005, output_file="space_tileset.png"):
    # Create a deep, dark space background (almost black, slight midnight blue tint)
    bg_color = (5, 5, 10)
    img = Image.new('RGB', (width, height), color=bg_color)
    draw = ImageDraw.Draw(img)

    num_stars = int(width * height * star_density)

    star_colors = [
        (255, 255, 255),  # Pure white
        (200, 220, 255),  # Ice blue
        (255, 230, 200),  # Warm dwarf
        (150, 150, 150)   # Distant gray
    ]

    for _ in range(num_stars):
        x = random.randint(0, width - 1)
        y = random.randint(0, height - 1)
    
        color = random.choice(star_colors)
        size_roll = random.random()

        if size_roll > 0.99:
            # 2% chance for a "large" glowing star (2x2 or 3x3 cross)
            draw.point((x, y), fill=color)
            draw.point((x+1, y), fill=color)
            draw.point((x-1, y), fill=color)
            draw.point((x, y+1), fill=color)
            draw.point((x, y-1), fill=color)
        elif size_roll > 0.875:
            draw.point((x, y), fill=color)
            draw.point((x+1, y), fill=color)
            draw.point((x, y+1), fill=color)
        else:
            dimmed_color = tuple(int(c * 0.6) for c in color)
            draw.point((x, y), fill=dimmed_color)

    img.save(output_file)
    print(f"Successfully generated {output_file} with {num_stars} stars!")

if __name__ == "__main__":
    # 512x512 is a great standard power-of-two texture size for WebGPU
    generate_starfield(512, 512, star_density=0.0025)
