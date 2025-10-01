"""Math utilities"""

# Module implementation

# utils/math_utils.py
import numpy as np
from typing import List, Tuple, Optional, Union
import math
from scipy import stats
import logging

logger = logging.getLogger(__name__)

class MathUtils:
    """Advanced mathematical utilities"""
    
    @staticmethod
    def clamp(value: float, min_val: float, max_val: float) -> float:
        """Clamp value between min and max"""
        return max(min_val, min(max_val, value))
    
    @staticmethod
    def lerp(a: float, b: float, t: float) -> float:
        """Linear interpolation between a and b"""
        return a + (b - a) * t
    
    @staticmethod
    def smoothstep(edge0: float, edge1: float, x: float) -> float:
        """Smoothstep function for smooth transitions"""
        x = MathUtils.clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0)
        return x * x * (3 - 2 * x)
    
    @staticmethod
    def remap(value: float, from_min: float, from_max: float, 
             to_min: float, to_max: float) -> float:
        """Remap value from one range to another"""
        return to_min + (value - from_min) * (to_max - to_min) / (from_max - from_min)
    
    @staticmethod
    def normalize_vector(vector: np.ndarray) -> np.ndarray:
        """Normalize a vector to unit length"""
        norm = np.linalg.norm(vector)
        if norm == 0:
            return vector
        return vector / norm
    
    @staticmethod
    def calculate_angle(v1: np.ndarray, v2: np.ndarray) -> float:
        """Calculate angle between two vectors in radians"""
        v1_norm = MathUtils.normalize_vector(v1)
        v2_norm = MathUtils.normalize_vector(v2)
        dot_product = np.clip(np.dot(v1_norm, v2_norm), -1.0, 1.0)
        return np.arccos(dot_product)

class Statistics:
    """Statistical analysis utilities"""
    
    @staticmethod
    def mean_confidence_interval(data: List[float], confidence: float = 0.95) -> Tuple[float, float]:
        """Calculate mean and confidence interval"""
        if not data:
            return 0.0, 0.0
        
        array = np.array(data)
        n = len(array)
        mean = np.mean(array)
        sem = stats.sem(array)
        h = sem * stats.t.ppf((1 + confidence) / 2., n - 1)
        return mean, h
    
    @staticmethod
    def outlier_detection(data: List[float], method: str = 'iqr') -> List[float]:
        """Detect outliers in data"""
        if not data:
            return []
        
        array = np.array(data)
        
        if method == 'iqr':
            # Interquartile range method
            Q1 = np.percentile(array, 25)
            Q3 = np.percentile(array, 75)
            IQR = Q3 - Q1
            lower_bound = Q1 - 1.5 * IQR
            upper_bound = Q3 + 1.5 * IQR
            outliers = array[(array < lower_bound) | (array > upper_bound)]
        
        elif method == 'zscore':
            # Z-score method
            z_scores = np.abs(stats.zscore(array))
            outliers = array[z_scores > 3]
        
        else:
            outliers = np.array([])
        
        return outliers.tolist()
    
    @staticmethod
    def moving_average(data: List[float], window: int) -> List[float]:
        """Calculate moving average"""
        if len(data) < window:
            return data
        
        return np.convolve(data, np.ones(window)/window, mode='valid').tolist()
    
    @staticmethod
    def exponential_smoothing(data: List[float], alpha: float) -> List[float]:
        """Exponential smoothing of time series data"""
        if not data:
            return []
        
        smoothed = [data[0]]
        for i in range(1, len(data)):
            smoothed.append(alpha * data[i] + (1 - alpha) * smoothed[i-1])
        
        return smoothed

class VectorCalculus:
    """Vector calculus operations"""
    
    @staticmethod
    def gradient(field: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """Calculate gradient of a 2D scalar field"""
        grad_y, grad_x = np.gradient(field)
        return grad_x, grad_y
    
    @staticmethod
    def divergence(vector_field_x: np.ndarray, vector_field_y: np.ndarray) -> np.ndarray:
        """Calculate divergence of a 2D vector field"""
        div_x, _ = np.gradient(vector_field_x)
        _, div_y = np.gradient(vector_field_y)
        return div_x + div_y
    
    @staticmethod
    def curl(vector_field_x: np.ndarray, vector_field_y: np.ndarray) -> np.ndarray:
        """Calculate curl of a 2D vector field"""
        _, curl_x = np.gradient(vector_field_y)
        curl_y, _ = np.gradient(vector_field_x)
        return curl_x - curl_y

class MatrixOperations:
    """Advanced matrix operations"""
    
    @staticmethod
    def matrix_power(matrix: np.ndarray, power: int) -> np.ndarray:
        """Calculate matrix power"""
        result = np.eye(matrix.shape[0])
        for _ in range(power):
            result = np.dot(result, matrix)
        return result
    
    @staticmethod
    def is_positive_definite(matrix: np.ndarray) -> bool:
        """Check if matrix is positive definite"""
        try:
            np.linalg.cholesky(matrix)
            return True
        except np.linalg.LinAlgError:
            return False
    
    @staticmethod
    def condition_number(matrix: np.ndarray) -> float:
        """Calculate condition number of matrix"""
        return np.linalg.cond(matrix)