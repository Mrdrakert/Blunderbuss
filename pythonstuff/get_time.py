import numpy as np
import matplotlib.pyplot as plt

# Define the get_time_for_move function
def get_time_for_move(time_left):
    y = 10000.0  # Initialize y as a float

    if time_left < 600000:
        y = (
            0.00000000000009074 * time_left ** 3 
            - 0.00000007166 * time_left ** 2 
            + 0.02666 * time_left 
            + 200.0
        )

    if time_left <10000:
        y = time_left / 22.0

    if time_left == 0:
        y = 4000.0

    time_for_move = int(y)
    return time_for_move

# Generate time_left values from 0 to 600000
time_left_values = np.linspace(0, 800000, 2000)

# Compute time_for_move for each time_left value
time_for_move_values = [get_time_for_move(t) for t in time_left_values]

# Plot the function
plt.figure(figsize=(10, 6))
plt.plot(time_left_values, time_for_move_values, label='get_time_for_move(time_left)', color='blue')
plt.title('Plot of get_time_for_move(time_left) from 0 to 600000')
plt.xlabel('time_left')
plt.ylabel('time_for_move')
plt.grid(True)
plt.legend()
plt.show()
