import os
from PIL import Image

def build_atlas():
  asset_order = [
    "space_tileset.png", # Slot 0: Background
    "light_tier.png",   # Slot 1: Player
    "mid_tier.png",    # Slot 2: Enemy (Placeholder)
    "heavy_tier.png",    # Slot 3: UI/Particles (Placeholder)
    "recon_drone.png"
  ]

  raw_dir = "assets/raw"
  output_file = "assets/atlas.png"

  columns = 4
  slot_size = 512
  atlas_size = slot_size * columns
  atlas = Image.new('RGBA', (atlas_size, atlas_size), (0, 0, 0, 0))

  print("--- Packing Texture Atlas ---")
  for index, filename in enumerate(asset_order):
    filepath = os.path.join(raw_dir, filename)
    row = index // columns
    col = index % columns
    x = col * slot_size
    y = row * slot_size

    if os.path.exists(filepath):
      img = Image.open(filepath).convert('RGBA')
      img = img.resize((slot_size, slot_size)) 
      atlas.paste(img, (x, y))
      print(f"[{index}] Packed {filename} at ({x}, {y})")
    else:
      print(f"[{index}] WARNING: {filename} not found. Leaving slot blank.")

  atlas.save(output_file)
  print(f"Successfully generated {output_file}!")

if __name__ == "__main__":
  build_atlas()
