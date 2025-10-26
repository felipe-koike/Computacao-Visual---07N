import tensorflow as tf
import pathlib
import os
from tensorflow.keras.callbacks import EarlyStopping

print("--- FASE 2: TREINAMENTO DO MODELO ---")

print("Carregando dataset local...")

base_path = pathlib.Path('.') 
data_dir = base_path / 'dataset' / 'Real Dataset'

if not data_dir.exists():
    print(f"Erro: O diretório do dataset '{data_dir}' não foi encontrado.")
    print("Verifique se a pasta 'dataset' está no mesmo local que este script.")
    exit()

batch_size = 32
img_height = 180
img_width = 180

print("Carregando dados de treinamento (de 'dataset/Real Dataset/train')...")
train_ds = tf.keras.utils.image_dataset_from_directory(
  data_dir / 'train',

  seed=123,
  image_size=(img_height, img_width),
  batch_size=batch_size)

print("Carregando dados de validação (de 'dataset/Real Dataset/validation')...")
val_ds = tf.keras.utils.image_dataset_from_directory(
  data_dir / 'validation', 
  image_size=(img_height, img_width),
  batch_size=batch_size)

class_names = train_ds.class_names
print(f"Classes encontradas: {class_names}")
num_classes = len(class_names)


print("\nConstruindo modelo CNN do zero...")

model = tf.keras.Sequential([
  tf.keras.layers.Rescaling(1./255, input_shape=(img_height, img_width, 3)),
  tf.keras.layers.RandomFlip("horizontal"),
  tf.keras.layers.RandomRotation(0.1),
  tf.keras.layers.RandomZoom(0.1),

  # Bloco 1
  tf.keras.layers.Conv2D(32, (3, 3), activation='relu', padding='same'),
  tf.keras.layers.MaxPooling2D((2, 2)), 

  # Bloco 2
  tf.keras.layers.Conv2D(64, (3, 3), activation='relu', padding='same'),
  tf.keras.layers.MaxPooling2D((2, 2)),

  # Bloco 3
  tf.keras.layers.Conv2D(128, (3, 3), activation='relu', padding='same'),
  tf.keras.layers.MaxPooling2D((2, 2)),

  # Bloco 4
  tf.keras.layers.Conv2D(256, (3, 3), activation='relu', padding='same'),
  tf.keras.layers.MaxPooling2D((2, 2)),
  
  tf.keras.layers.Flatten(),

  # Camadas Totalmente Conectadas
  tf.keras.layers.Dense(512, activation='relu'),
  tf.keras.layers.Dropout(0.5),

  # Camada de saída
  tf.keras.layers.Dense(num_classes)
])


model.compile(optimizer='adam',
              loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
              metrics=['accuracy'])

model.summary()



print("\nIniciando o treinamento...")

epochs = 10 

early_stopping = EarlyStopping(
    monitor='val_loss',
    patience=5,
    restore_best_weights=True
)

history = model.fit(
  train_ds,
  validation_data=val_ds,
  epochs=epochs,
  callbacks=[early_stopping]
)


print("\nTreinamento concluído! Salvando o modelo...")

model.save('./banana_classifier_model.h5')
print("Modelo salvo como './banana_classifier_model.h5'")

with open('./class_names.txt', 'w') as f:
    for item in class_names:
        f.write("%s\n" % item)
print("Nomes das classes salvos em './class_names.txt'")