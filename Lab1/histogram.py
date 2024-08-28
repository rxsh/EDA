import os
import matplotlib.pyplot as plt
import numpy as np

try:
    with open('C:/Users/cesar/Desktop/DS-EDA/Lab1/distancias.txt', 'r') as file:
        distancias = [float(line.strip()) for line in file]
except FileNotFoundError:
    print("El archivo 'distancias.txt' no se encuentra en el directorio.")
    exit()

distancias_array = np.array(distancias)

min_distancia = 0.0
max_distancia = np.max(distancias_array)

plt.figure(figsize=(10, 6))
plt.hist(distancias_array, bins=20, alpha=0.75, color='skyblue', rwidth=0.8, edgecolor='black')
plt.title('Histograma de la distancias con dimension 5000')
plt.xlabel('Distancia')
plt.ylabel('Frecuencia')
plt.xlim(0,30) # Viendo el rango de numeros que crea editamos de que punto a que punto va para que se vea en el gráfico
plt.ylim(0,700)
plt.grid(True)
plt.show()
