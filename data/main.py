from PIL import Image
import os

# Lista atualizada das imagens para redimensionar
image_names = [
    "theme_thumbnail.png",
    "theme_box.png",
    "scroll_arrow_up.png",
    "scroll_arrow_down.png",
    "scrollbar_track.png"
]

# Fator de escala (1024x768 → 640x480)
SCALE_FACTOR = 0.625

# Pastas
input_folder = "./"
output_folder = "./resized/"

# Criar a pasta de saída se não existir
os.makedirs(output_folder, exist_ok=True)

# Função para redimensionar
def resize_image(file_path, output_path):
    img = Image.open(file_path)
    new_size = (int(img.width * SCALE_FACTOR), int(img.height * SCALE_FACTOR))
    resized_img = img.resize(new_size, Image.LANCZOS)
    resized_img.save(output_path)
    print(f"✅ {os.path.basename(file_path)} redimensionada para {new_size}")

# Processar todas as imagens
for name in image_names:
    input_path = os.path.join(input_folder, name)
    output_path = os.path.join(output_folder, name)

    if os.path.exists(input_path):
        resize_image(input_path, output_path)
    else:
        print(f"⚠️ {name} não encontrado.")
