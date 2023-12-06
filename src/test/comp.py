import matplotlib.pyplot as plt


X = [10, 100, 1000, 10000]

ser_thread = [0.002, 0.016, 0.125, 1.477]
ng_thread = [0.024, 0.054, 0.330, 2.825]

# Plotting both the curves simultaneously 
plt.plot(X, ser_thread, color='r', marker='v', label='thread pool + poll') 
plt.plot(X, ng_thread, color='g', linestyle='--', marker='o', label='nginx') 
  
# Naming the x-axis, y-axis and the whole graph 
plt.xlabel("Количество запросов") 
plt.ylabel("Время работы (сек.)") 
plt.xscale('symlog')
plt.title("Время работы сервера на 5 клиентах") 
  
# Adding legend, which helps us recognize the curve according to it's color 
plt.legend() 
  
# To load the display window 
plt.show() 