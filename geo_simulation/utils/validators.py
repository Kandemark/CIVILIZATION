"""Data validators"""

# Module implementation

# utils/validators.py
from typing import Any, Dict, List, Optional, Callable, Union
import re
from numbers import Number
import logging

logger = logging.getLogger(__name__)

class InputValidator:
    """Input validation utilities"""
    
    @staticmethod
    def validate_email(email: str) -> bool:
        """Validate email address format"""
        pattern = r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
        return bool(re.match(pattern, email))
    
    @staticmethod
    def validate_phone(phone: str) -> bool:
        """Validate phone number format"""
        # Simple validation for common formats
        pattern = r'^[\+]?[0-9\s\-\(\)]{10,}$'
        return bool(re.match(pattern, phone))
    
    @staticmethod
    def validate_url(url: str) -> bool:
        """Validate URL format"""
        pattern = r'^https?://[^\s/$.?#].[^\s]*$'
        return bool(re.match(pattern, url))
    
    @staticmethod
    def validate_number(value: Any, min_val: Optional[Number] = None, 
                       max_val: Optional[Number] = None) -> bool:
        """Validate number within range"""
        if not isinstance(value, Number):
            return False
        
        if min_val is not None and value < min_val:
            return False
        
        if max_val is not None and value > max_val:
            return False
        
        return True
    
    @staticmethod
    def validate_string(value: Any, min_length: Optional[int] = None,
                      max_length: Optional[int] = None, pattern: Optional[str] = None) -> bool:
        """Validate string with length and pattern constraints"""
        if not isinstance(value, str):
            return False
        
        if min_length is not None and len(value) < min_length:
            return False
        
        if max_length is not None and len(value) > max_length:
            return False
        
        if pattern is not None and not re.match(pattern, value):
            return False
        
        return True

class SchemaValidator:
    """JSON schema validation"""
    
    @staticmethod
    def validate_schema(data: Dict[str, Any], schema: Dict[str, Any]) -> List[str]:
        """Validate data against schema, return list of errors"""
        errors = []
        
        for field, rules in schema.items():
            if field not in data and rules.get('required', False):
                errors.append(f"Missing required field: {field}")
                continue
            
            if field in data:
                value = data[field]
                field_errors = SchemaValidator._validate_field(value, rules, field)
                errors.extend(field_errors)
        
        return errors
    
    @staticmethod
    def _validate_field(value: Any, rules: Dict[str, Any], field_name: str) -> List[str]:
        """Validate a single field against rules"""
        errors = []
        
        # Type validation
        expected_type = rules.get('type')
        if expected_type and not isinstance(value, expected_type):
            errors.append(f"Field {field_name}: expected {expected_type}, got {type(value)}")
        
        # Range validation for numbers
        if isinstance(value, Number):
            if 'min' in rules and value < rules['min']:
                errors.append(f"Field {field_name}: value {value} below minimum {rules['min']}")
            if 'max' in rules and value > rules['max']:
                errors.append(f"Field {field_name}: value {value} above maximum {rules['max']}")
        
        # Length validation for strings
        if isinstance(value, str):
            if 'min_length' in rules and len(value) < rules['min_length']:
                errors.append(f"Field {field_name}: length {len(value)} below minimum {rules['min_length']}")
            if 'max_length' in rules and len(value) > rules['max_length']:
                errors.append(f"Field {field_name}: length {len(value)} above maximum {rules['max_length']}")
        
        # Pattern validation
        if 'pattern' in rules and isinstance(value, str):
            if not re.match(rules['pattern'], value):
                errors.append(f"Field {field_name}: value does not match pattern {rules['pattern']}")
        
        # Custom validator
        if 'validator' in rules and callable(rules['validator']):
            if not rules['validator'](value):
                errors.append(f"Field {field_name}: custom validation failed")
        
        return errors

class RangeValidator:
    """Range validation utilities"""
    
    @staticmethod
    def in_range(value: Number, min_val: Number, max_val: Number) -> bool:
        """Check if value is in range [min_val, max_val]"""
        return min_val <= value <= max_val
    
    @staticmethod
    def in_range_exclusive(value: Number, min_val: Number, max_val: Number) -> bool:
        """Check if value is in range (min_val, max_val)"""
        return min_val < value < max_val
    
    @staticmethod
    def percentage(value: Number) -> bool:
        """Check if value is a valid percentage (0-100)"""
        return 0 <= value <= 100
    
    @staticmethod
    def probability(value: Number) -> bool:
        """Check if value is a valid probability (0-1)"""
        return 0 <= value <= 1
    
    @staticmethod
    def positive(value: Number) -> bool:
        """Check if value is positive"""
        return value > 0
    
    @staticmethod
    def non_negative(value: Number) -> bool:
        """Check if value is non-negative"""
        return value >= 0

# Composite validators
class CompositeValidator:
    """Combine multiple validators"""
    
    def __init__(self):
        self.validators: List[Callable[[Any], bool]] = []
    
    def add_validator(self, validator: Callable[[Any], bool]):
        """Add a validator"""
        self.validators.append(validator)
    
    def validate(self, value: Any) -> bool:
        """Validate value against all validators"""
        return all(validator(value) for validator in self.validators)
    
    def validate_with_errors(self, value: Any) -> List[str]:
        """Validate and return error messages"""
        errors = []
        for validator in self.validators:
            try:
                if not validator(value):
                    errors.append(f"Validation failed: {validator.__name__}")
            except Exception as e:
                errors.append(f"Validation error: {e}")
        return errors