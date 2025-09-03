"""JSON serialization"""

# Module implementation

"""
JSON serialization system for game save/load functionality.
"""

import json
from typing import Dict, Any
from datetime import datetime
import gzip
import base64

class JsonSerializer:
    """Handles JSON serialization and deserialization for game data."""
    
    def __init__(self, compress: bool = True, indent: int = 2):
        self.compress = compress
        self.indent = indent
    
    def save_to_file(self, data: Dict[str, Any], filename: str):
        """Save data to a JSON file, optionally with compression."""
        try:
            # Convert non-serializable objects
            serializable_data = self._make_serializable(data)
            
            if self.compress:
                # Compress JSON data
                json_str = json.dumps(serializable_data, indent=self.indent)
                compressed_data = gzip.compress(json_str.encode('utf-8'))
                
                with open(filename, 'wb') as f:
                    f.write(compressed_data)
            else:
                # Regular JSON
                with open(filename, 'w', encoding='utf-8') as f:
                    json.dump(serializable_data, f, indent=self.indent, ensure_ascii=False)
                    
        except Exception as e:
            raise ValueError(f"Error saving to {filename}: {e}")
    
    def load_from_file(self, filename: str) -> Dict[str, Any]:
        """Load data from a JSON file, handling compression if needed."""
        try:
            if self.compress:
                # Load compressed data
                with open(filename, 'rb') as f:
                    compressed_data = f.read()
                
                json_str = gzip.decompress(compressed_data).decode('utf-8')
                return json.loads(json_str)
            else:
                # Load regular JSON
                with open(filename, 'r', encoding='utf-8') as f:
                    return json.load(f)
                    
        except Exception as e:
            raise ValueError(f"Error loading from {filename}: {e}")
    
    def _make_serializable(self, data: Any) -> Any:
        """Convert non-serializable objects to serializable formats."""
        if isinstance(data, dict):
            return {k: self._make_serializable(v) for k, v in data.items()}
        elif isinstance(data, list):
            return [self._make_serializable(item) for item in data]
        elif isinstance(data, (str, int, float, bool, type(None))):
            return data
        elif hasattr(data, 'to_dict'):
            return data.to_dict()
        elif hasattr(data, 'isoformat'):  # datetime objects
            return data.isoformat()
        else:
            # Try to convert to string as fallback
            try:
                return str(data)
            except:
                return None
    
    def to_base64(self, data: Dict[str, Any]) -> str:
        """Convert data to base64-encoded string for compact storage."""
        json_str = json.dumps(self._make_serializable(data))
        compressed = gzip.compress(json_str.encode('utf-8'))
        return base64.b64encode(compressed).decode('ascii')
    
    def from_base64(self, base64_str: str) -> Dict[str, Any]:
        """Convert base64-encoded string back to data."""
        compressed = base64.b64decode(base64_str.encode('ascii'))
        json_str = gzip.decompress(compressed).decode('utf-8')
        return json.loads(json_str)