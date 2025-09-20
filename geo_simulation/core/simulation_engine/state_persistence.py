"""Save/load optimization"""

# Module implementation

# simulation_engine/state_persistence.py
import json
import pickle
from typing import Dict, Any, Optional, List
from dataclasses import dataclass
import logging
from pathlib import Path
import zlib
from datetime import datetime

logger = logging.getLogger(__name__)

@dataclass
class SaveGameInfo:
    filename: str
    timestamp: datetime
    version: str
    description: str
    size: int

class StatePersistence:
    """
    Handles saving and loading simulation state with versioning and compression.
    """
    
    def __init__(self, save_dir: str = "saves"):
        self.save_dir = Path(save_dir)
        self.save_dir.mkdir(exist_ok=True)
        self.current_version = "1.0.0"
        self.compression_enabled = True
        
    def save_state(self, state: Dict[str, Any], filename: str, 
                  description: str = "") -> bool:
        """Save simulation state to file"""
        try:
            # Add metadata
            state_with_meta = {
                'version': self.current_version,
                'timestamp': datetime.now().isoformat(),
                'description': description,
                'data': state
            }
            
            # Serialize
            serialized = pickle.dumps(state_with_meta)
            
            # Compress if enabled
            if self.compression_enabled:
                serialized = zlib.compress(serialized)
                
            # Write to file
            filepath = self.save_dir / filename
            with open(filepath, 'wb') as f:
                f.write(serialized)
                
            logger.info(f"State saved to {filepath}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to save state: {e}")
            return False
            
    def load_state(self, filename: str) -> Optional[Dict[str, Any]]:
        """Load simulation state from file"""
        try:
            filepath = self.save_dir / filename
            
            if not filepath.exists():
                logger.error(f"Save file not found: {filepath}")
                return None
                
            with open(filepath, 'rb') as f:
                data = f.read()
                
            # Decompress if compressed
            try:
                data = zlib.decompress(data)
                compressed = True
            except:
                compressed = False
                
            # Deserialize
            state_with_meta = pickle.loads(data)
            
            # Check version compatibility
            if state_with_meta['version'] != self.current_version:
                logger.warning(f"Loading state from different version: {state_with_meta['version']}")
                
            logger.info(f"State loaded from {filepath} (compressed: {compressed})")
            return state_with_meta['data']
            
        except Exception as e:
            logger.error(f"Failed to load state: {e}")
            return None
            
    def list_saves(self) -> List[SaveGameInfo]:
        """List all available save games"""
        saves = []
        
        for filepath in self.save_dir.glob("*.save"):
            try:
                with open(filepath, 'rb') as f:
                    data = f.read()
                    
                # Try to decompress to read metadata
                try:
                    data = zlib.decompress(data)
                except:
                    pass
                    
                state_with_meta = pickle.loads(data)
                
                save_info = SaveGameInfo(
                    filename=filepath.name,
                    timestamp=datetime.fromisoformat(state_with_meta['timestamp']),
                    version=state_with_meta['version'],
                    description=state_with_meta['description'],
                    size=filepath.stat().st_size
                )
                
                saves.append(save_info)
                
            except Exception as e:
                logger.warning(f"Could not read save file metadata for {filepath}: {e}")
                
        return sorted(saves, key=lambda x: x.timestamp, reverse=True)
        
    def export_to_json(self, filename: str, state: Dict[str, Any]) -> bool:
        """Export state to human-readable JSON format"""
        try:
            # Convert to JSON-serializable format
            def convert_for_json(obj):
                if hasattr(obj, '__dict__'):
                    return obj.__dict__
                return str(obj)
                
            json_data = json.dumps(state, default=convert_for_json, indent=2)
            
            filepath = self.save_dir / f"{filename}.json"
            with open(filepath, 'w') as f:
                f.write(json_data)
                
            logger.info(f"State exported to JSON: {filepath}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to export to JSON: {e}")
            return False
            
    def enable_compression(self, enabled: bool = True) -> None:
        """Enable or disable compression for saves"""
        self.compression_enabled = enabled
        logger.info(f"Compression {'enabled' if enabled else 'disabled'}")