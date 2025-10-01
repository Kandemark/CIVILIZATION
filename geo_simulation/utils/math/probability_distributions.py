"""Probability distributions"""

# Module implementation

# utils/math/probability_distributions.py

"""
Probability Distributions for Statistical Modeling

Implements various probability distributions for simulation randomness and statistics.
"""

import numpy as np
from typing import Dict, List, Optional, Tuple, Union, Callable
from dataclasses import dataclass, field
import math
import logging
from abc import ABC, abstractmethod

logger = logging.getLogger(__name__)


class ProbabilityDistribution(ABC):
    """Abstract base class for probability distributions."""
    
    @abstractmethod
    def pdf(self, x: float) -> float:
        """Probability density function."""
        pass
    
    @abstractmethod
    def cdf(self, x: float) -> float:
        """Cumulative distribution function."""
        pass
    
    @abstractmethod
    def sample(self, n: int = 1) -> Union[float, List[float]]:
        """Generate random samples from the distribution."""
        pass
    
    @abstractmethod
    def mean(self) -> float:
        """Calculate the mean of the distribution."""
        pass
    
    @abstractmethod
    def variance(self) -> float:
        """Calculate the variance of the distribution."""
        pass


@dataclass
class NormalDistribution(ProbabilityDistribution):
    """Normal (Gaussian) distribution."""
    mean: float = 0.0
    std: float = 1.0
    
    def pdf(self, x: float) -> float:
        """Probability density function."""
        return (1.0 / (self.std * math.sqrt(2 * math.pi))) * \
               math.exp(-0.5 * ((x - self.mean) / self.std) ** 2)
    
    def cdf(self, x: float) -> float:
        """Cumulative distribution function (approximation)."""
        # Abramowitz and Stegun approximation
        z = (x - self.mean) / self.std
        t = 1.0 / (1.0 + 0.2316419 * abs(z))
        poly = t * (0.319381530 + t * (-0.356563782 + t * (1.781477937 + 
              t * (-1.821255978 + t * 1.330274429))))
        cdf = 1.0 - self.pdf(x) * poly
        
        return cdf if z >= 0 else 1.0 - cdf
    
    def sample(self, n: int = 1) -> Union[float, List[float]]:
        """Generate random samples using Box-Muller transform."""
        if n == 1:
            # Box-Muller for single sample
            u1, u2 = np.random.random(), np.random.random()
            z0 = math.sqrt(-2.0 * math.log(u1)) * math.cos(2.0 * math.pi * u2)
            return z0 * self.std + self.mean
        else:
            # Multiple samples
            samples = []
            for _ in range((n + 1) // 2):
                u1, u2 = np.random.random(), np.random.random()
                z0 = math.sqrt(-2.0 * math.log(u1)) * math.cos(2.0 * math.pi * u2)
                z1 = math.sqrt(-2.0 * math.log(u1)) * math.sin(2.0 * math.pi * u2)
                samples.extend([z0 * self.std + self.mean, z1 * self.std + self.mean])
            
            return samples[:n]
    
    def mean(self) -> float:
        return self.mean
    
    def variance(self) -> float:
        return self.std ** 2


@dataclass
class PoissonDistribution(ProbabilityDistribution):
    """Poisson distribution for counting events."""
    lambda_: float  # Rate parameter
    
    def pdf(self, k: int) -> float:
        """Probability mass function."""
        if k < 0:
            return 0.0
        return (self.lambda_ ** k * math.exp(-self.lambda_)) / math.factorial(k)
    
    def cdf(self, k: int) -> float:
        """Cumulative distribution function."""
        if k < 0:
            return 0.0
        
        total = 0.0
        for i in range(k + 1):
            total += self.pdf(i)
        
        return total
    
    def sample(self, n: int = 1) -> Union[int, List[int]]:
        """Generate random samples using Knuth's algorithm."""
        if n == 1:
            L = math.exp(-self.lambda_)
            k = 0
            p = 1.0
            
            while p > L:
                k += 1
                p *= np.random.random()
            
            return k - 1
        else:
            return [self.sample() for _ in range(n)]
    
    def mean(self) -> float:
        return self.lambda_
    
    def variance(self) -> float:
        return self.lambda_


@dataclass
class ExponentialDistribution(ProbabilityDistribution):
    """Exponential distribution for time between events."""
    lambda_: float  # Rate parameter
    
    def pdf(self, x: float) -> float:
        """Probability density function."""
        if x < 0:
            return 0.0
        return self.lambda_ * math.exp(-self.lambda_ * x)
    
    def cdf(self, x: float) -> float:
        """Cumulative distribution function."""
        if x < 0:
            return 0.0
        return 1.0 - math.exp(-self.lambda_ * x)
    
    def sample(self, n: int = 1) -> Union[float, List[float]]:
        """Generate random samples using inverse transform."""
        if n == 1:
            return -math.log(1.0 - np.random.random()) / self.lambda_
        else:
            return [-math.log(1.0 - np.random.random()) / self.lambda_ for _ in range(n)]
    
    def mean(self) -> float:
        return 1.0 / self.lambda_
    
    def variance(self) -> float:
        return 1.0 / (self.lambda_ ** 2)


@dataclass
class BinomialDistribution(ProbabilityDistribution):
    """Binomial distribution for success/failure trials."""
    n: int  # Number of trials
    p: float  # Success probability
    
    def pdf(self, k: int) -> float:
        """Probability mass function."""
        if k < 0 or k > self.n:
            return 0.0
        
        # Using logarithms to avoid large numbers
        log_pmf = (math.lgamma(self.n + 1) - math.lgamma(k + 1) - math.lgamma(self.n - k + 1) +
                   k * math.log(self.p) + (self.n - k) * math.log(1 - self.p))
        
        return math.exp(log_pmf)
    
    def cdf(self, k: int) -> float:
        """Cumulative distribution function."""
        if k < 0:
            return 0.0
        if k >= self.n:
            return 1.0
        
        total = 0.0
        for i in range(k + 1):
            total += self.pdf(i)
        
        return total
    
    def sample(self, n: int = 1) -> Union[int, List[int]]:
        """Generate random samples."""
        if n == 1:
            # Sum of n Bernoulli trials
            return sum(1 for _ in range(self.n) if np.random.random() < self.p)
        else:
            return [self.sample() for _ in range(n)]
    
    def mean(self) -> float:
        return self.n * self.p
    
    def variance(self) -> float:
        return self.n * self.p * (1 - self.p)


@dataclass
class CustomDistribution(ProbabilityDistribution):
    """Custom distribution defined by a probability density function."""
    pdf_function: Callable[[float], float]
    support: Tuple[float, float] = (-np.inf, np.inf)
    normalization_constant: float = 1.0
    
    def __post_init__(self):
        """Calculate normalization constant if not provided."""
        if self.normalization_constant == 1.0:
            # Simple numerical integration for normalization
            a, b = self.support
            if np.isfinite(a) and np.isfinite(b):
                x_vals = np.linspace(a, b, 1000)
                pdf_vals = [self.pdf_function(x) for x in x_vals]
                area = np.trapz(pdf_vals, x_vals)
                if area > 0:
                    self.normalization_constant = area
    
    def pdf(self, x: float) -> float:
        """Probability density function."""
        if x < self.support[0] or x > self.support[1]:
            return 0.0
        return self.pdf_function(x) / self.normalization_constant
    
    def cdf(self, x: float) -> float:
        """Cumulative distribution function (numerical integration)."""
        if x <= self.support[0]:
            return 0.0
        if x >= self.support[1]:
            return 1.0
        
        a = self.support[0]
        x_vals = np.linspace(a, x, 1000)
        pdf_vals = [self.pdf(x_val) for x_val in x_vals]
        return np.trapz(pdf_vals, x_vals)
    
    def sample(self, n: int = 1) -> Union[float, List[float]]:
        """Generate random samples using rejection sampling."""
        a, b = self.support
        if not (np.isfinite(a) and np.isfinite(b)):
            raise ValueError("Rejection sampling requires finite support")
        
        # Find maximum of PDF for rejection sampling
        x_vals = np.linspace(a, b, 1000)
        pdf_vals = [self.pdf(x) for x in x_vals]
        max_pdf = max(pdf_vals)
        
        def sample_single():
            while True:
                x = a + (b - a) * np.random.random()
                y = max_pdf * np.random.random()
                if y <= self.pdf(x):
                    return x
        
        if n == 1:
            return sample_single()
        else:
            return [sample_single() for _ in range(n)]
    
    def mean(self) -> float:
        """Calculate mean (numerical integration)."""
        a, b = self.support
        if not (np.isfinite(a) and np.isfinite(b)):
            return float('nan')
        
        x_vals = np.linspace(a, b, 1000)
        pdf_vals = [self.pdf(x) for x in x_vals]
        mean_integrand = [x * pdf for x, pdf in zip(x_vals, pdf_vals)]
        return np.trapz(mean_integrand, x_vals)
    
    def variance(self) -> float:
        """Calculate variance (numerical integration)."""
        mean_val = self.mean()
        a, b = self.support
        if not (np.isfinite(a) and np.isfinite(b)):
            return float('nan')
        
        x_vals = np.linspace(a, b, 1000)
        pdf_vals = [self.pdf(x) for x in x_vals]
        var_integrand = [((x - mean_val) ** 2) * pdf for x, pdf in zip(x_vals, pdf_vals)]
        return np.trapz(var_integrand, x_vals)


def fit_distribution(data: List[float], dist_type: str) -> ProbabilityDistribution:
    """Fit a distribution to data."""
    data = np.array(data)
    
    if dist_type == "normal":
        mean = np.mean(data)
        std = np.std(data)
        return NormalDistribution(mean=mean, std=std)
    
    elif dist_type == "exponential":
        lambda_ = 1.0 / np.mean(data)
        return ExponentialDistribution(lambda_=lambda_)
    
    elif dist_type == "poisson":
        lambda_ = np.mean(data)
        return PoissonDistribution(lambda_=lambda_)
    
    else:
        raise ValueError(f"Unknown distribution type: {dist_type}")


def kolmogorov_smirnov_test(data: List[float], distribution: ProbabilityDistribution) -> float:
    """Kolmogorov-Smirnov test for goodness of fit."""
    data_sorted = sorted(data)
    n = len(data_sorted)
    
    # Empirical CDF
    ecdf = np.array([(i + 1) / n for i in range(n)])
    
    # Theoretical CDF
    tcdf = np.array([distribution.cdf(x) for x in data_sorted])
    
    # KS statistic
    d_plus = np.max(np.abs(ecdf - tcdf))
    d_minus = np.max(np.abs(tcdf - np.array([i / n for i in range(n)])))
    
    return max(d_plus, d_minus)




# --- IGNORE ---

# import numpy as np
# from typing import Dict, List, Optional, Union
# from dataclasses import dataclass
# from enum import Enum
# import logging

# logger = logging.getLogger(__name__)

# class DistributionType(Enum):
#     NORMAL = "normal"
#     UNIFORM = "uniform"
#     POISSON = "poisson"
#     BINOMIAL = "binomial"
#     EXPONENTIAL = "exponential"
#     LOG_NORMAL = "lognormal"

# @dataclass
# class ProbabilityDistribution:
#     """Base class for probability distributions"""
#     distribution_type: DistributionType
#     parameters: Dict[str, float]
    
#     def sample(self, size: int = 1) -> np.ndarray:
#         """Sample from the distribution"""
#         raise NotImplementedError("Subclasses must implement sample method")
    
#     def pdf(self, x: Union[float, np.ndarray]) -> Union[float, np.ndarray]:
#         """Probability density function"""
#         raise NotImplementedError("Subclasses must implement pdf method")
    
#     def cdf(self, x: Union[float, np.ndarray]) -> Union[float, np.ndarray]:
#         """Cumulative distribution function"""
#         raise NotImplementedError("Subclasses must implement cdf method")

# class NormalDistribution(ProbabilityDistribution):
#     """Normal (Gaussian) distribution"""
    
#     def __init__(self, mean: float = 0.0, std_dev: float = 1.0):
#         super().__init__(DistributionType.NORMAL, {'mean': mean, 'std_dev': std_dev})
#         self.mean = mean
#         self.std_dev = std_dev
    
#     def sample(self, size: int = 1) -> np.ndarray:
#         return np.random.normal(self.mean, self.std_dev, size)
    
#     def pdf(self, x: Union[float, np.ndarray]) -> Union[float, np.ndarray]:
#         return (1.0 / (self.std_dev * np.sqrt(2 * np.pi)) * 
#                 np.exp(-0.5 * ((x - self.mean) / self.std_dev) ** 2))
    
#     def cdf(self, x: Union[float, np.ndarray]) -> Union[float, np.ndarray]:
#         from scipy.special import erf
#         return 0.5 * (1 + erf((x - self.mean) / (self.std_dev * np.sqrt(2))))

# class PoissonDistribution(ProbabilityDistribution):
#     """Poisson distribution for counting events"""
    
#     def __init__(self, lam: float = 1.0):
#         super().__init__(DistributionType.POISSON, {'lambda': lam})
#         self.lam = lam
    
#     def sample(self, size: int = 1) -> np.ndarray:
#         return np.random.poisson(self.lam, size)
    
#     def pdf(self, k: Union[int, np.ndarray]) -> Union[float, np.ndarray]:
#         if isinstance(k, (int, np.integer)):
#             return (self.lam ** k * np.exp(-self.lam)) / np.math.factorial(k)
#         else:
#             return np.array([self.pdf(int(x)) for x in k])
    
#     def cdf(self, k: Union[int, np.ndarray]) -> Union[float, np.ndarray]:
#         from scipy.special import gammaincc
#         if isinstance(k, (int, np.integer)):
#             return 1.0 - gammaincc(k + 1, self.lam)
#         else:
#             return np.array([self.cdf(int(x)) for x in k])

# # Additional distribution implementations would follow similar patterns