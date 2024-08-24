import numpy as np
import matplotlib.pyplot as plt
from numpy.polynomial.polynomial import Polynomial

# Example data points
x = np.array([15000 ,40000, 60000])
y = np.array([700,1200, 2000])

# Fit a polynomial of degree 2 (quadratic)
p = Polynomial.fit(x, y, 4)

# Get the polynomial coefficients
coefs = p.convert().coef
print("Polynomial coefficients:", coefs)

# Plotting
plt.scatter(x, y, label='Data Points')
plt.plot(x, p(x), label='Fitted Polynomial')
plt.legend()
plt.show()
