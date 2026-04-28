import os
from PIL import Image

def build_atlas():
  asset_order = [
    "space_tileset.png",
    "light_tier.png",
    "mid_tier.png",
    "heavy_tier.png",
    "recon_drone.png",
    "tile_border.png",
    "enemy_tile.png",
    "tile_border_highlight.png",
    "enemy_tile_highlight.png"
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

    cell_x = col * slot_size
    cell_y = row * slot_size

    if os.path.exists(filepath):
      img = Image.open(filepath).convert('RGBA')

      try:
        resample_filter = Image.Resampling.LANCZOS
      except AttributeError:
        resample_filter = Image.LANCZOS

      img.thumbnail((slot_size, slot_size), resample_filter) 

      offset_x = cell_x + (slot_size - img.width) // 2
      offset_y = cell_y + (slot_size - img.height) // 2

      atlas.paste(img, (offset_x, offset_y), img)

      print(f"[{index}] Packed {filename} centered at ({offset_x}, {offset_y})")
    else:
      print(f"[{index}] WARNING: {filename} not found. Leaving slot blank.")

  atlas.save(output_file)
  print(f"Successfully generated {output_file}!")

if __name__ == "__main__":
  build_atlas()
