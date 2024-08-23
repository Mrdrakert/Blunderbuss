import numpy as np
import matplotlib.pyplot as plt
from numpy.polynomial.polynomial import Polynomial

# Example data points
x = np.array([0, 30000, 60000, 120000, 600000])
y = np.array([ 200, 1200, 2000, 3200, 10000])

# Fit a polynomial of degree 2 (quadratic)
p = Polynomial.fit(x, y, 3)

# Get the polynomial coefficients
coefs = p.convert().coef
print("Polynomial coefficients:", coefs)

# Plotting
plt.scatter(x, y, label='Data Points')
plt.plot(x, p(x), label='Fitted Polynomial')
plt.legend()
plt.show()
