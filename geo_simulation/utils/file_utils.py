"""File utilities"""

# Module implementation

# utils/file_utils.py
import json
import yaml
import pickle
import csv
from typing import Any, Dict, List, Optional, Union
from pathlib import Path
import logging
from datetime import datetime
import gzip
import zipfile

logger = logging.getLogger(__name__)

class JSONHandler:
    """JSON file handling utilities"""
    
    @staticmethod
    def load_json(file_path: Union[str, Path]) -> Any:
        """Load data from JSON file"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load JSON from {file_path}: {e}")
            raise
    
    @staticmethod
    def save_json(data: Any, file_path: Union[str, Path], 
                 indent: int = 2, ensure_ascii: bool = False) -> bool:
        """Save data to JSON file"""
        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=indent, ensure_ascii=ensure_ascii)
            logger.debug(f"Data saved to JSON: {file_path}")
            return True
        except Exception as e:
            logger.error(f"Failed to save JSON to {file_path}: {e}")
            return False
    
    @staticmethod
    def validate_json_schema(data: Dict, schema: Dict) -> bool:
        """Validate JSON data against schema (simplified)"""
        try:
            # Basic schema validation - in production, use jsonschema library
            required_fields = schema.get('required', [])
            for field in required_fields:
                if field not in data:
                    return False
            return True
        except Exception as e:
            logger.error(f"Schema validation failed: {e}")
            return False

class YAMLConfig:
    """YAML configuration file handler"""
    
    @staticmethod
    def load_config(file_path: Union[str, Path]) -> Dict[str, Any]:
        """Load configuration from YAML file"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                return yaml.safe_load(f)
        except Exception as e:
            logger.error(f"Failed to load YAML config from {file_path}: {e}")
            raise
    
    @staticmethod
    def save_config(config: Dict[str, Any], file_path: Union[str, Path]) -> bool:
        """Save configuration to YAML file"""
        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                yaml.dump(config, f, default_flow_style=False, allow_unicode=True)
            logger.debug(f"Config saved to YAML: {file_path}")
            return True
        except Exception as e:
            logger.error(f"Failed to save YAML config to {file_path}: {e}")
            return False
    
    @staticmethod
    def update_config(file_path: Union[str, Path], updates: Dict[str, Any]) -> bool:
        """Update configuration file with new values"""
        try:
            config = YAMLConfig.load_config(file_path)
            config.update(updates)
            return YAMLConfig.save_config(config, file_path)
        except Exception as e:
            logger.error(f"Failed to update config {file_path}: {e}")
            return False

class SaveGameManager:
    """Save game management system"""
    
    def __init__(self, save_dir: str = "saves"):
        self.save_dir = Path(save_dir)
        self.save_dir.mkdir(exist_ok=True)
    
    def create_save(self, game_data: Dict[str, Any], save_name: str, description: str = "") -> bool:
        """Create a new save game"""
        save_data = {
            'metadata': {
                'name': save_name,
                'description': description,
                'timestamp': datetime.now().isoformat(),
                'version': '1.0.0'
            },
            'game_data': game_data
        }
        
        save_path = self.save_dir / f"{save_name}.save"
        
        try:
            with open(save_path, 'wb') as f:
                pickle.dump(save_data, f)
            logger.info(f"Game saved: {save_name}")
            return True
        except Exception as e:
            logger.error(f"Failed to create save {save_name}: {e}")
            return False
    
    def load_save(self, save_name: str) -> Optional[Dict[str, Any]]:
        """Load a save game"""
        save_path = self.save_dir / f"{save_name}.save"
        
        try:
            with open(save_path, 'rb') as f:
                save_data = pickle.load(f)
            logger.info(f"Game loaded: {save_name}")
            return save_data['game_data']
        except Exception as e:
            logger.error(f"Failed to load save {save_name}: {e}")
            return None
    
    def list_saves(self) -> List[Dict[str, str]]:
        """List all available save games"""
        saves = []
        
        for save_file in self.save_dir.glob("*.save"):
            try:
                with open(save_file, 'rb') as f:
                    save_data = pickle.load(f)
                saves.append({
                    'name': save_file.stem,
                    'description': save_data['metadata'].get('description', ''),
                    'timestamp': save_data['metadata'].get('timestamp', ''),
                    'version': save_data['metadata'].get('version', '')
                })
            except Exception as e:
                logger.warning(f"Could not read save file {save_file}: {e}")
        
        # Sort by timestamp (newest first)
        saves.sort(key=lambda x: x.get('timestamp', ''), reverse=True)
        return saves
    
    def delete_save(self, save_name: str) -> bool:
        """Delete a save game"""
        save_path = self.save_dir / f"{save_name}.save"
        
        try:
            if save_path.exists():
                save_path.unlink()
                logger.info(f"Save deleted: {save_name}")
                return True
            return False
        except Exception as e:
            logger.error(f"Failed to delete save {save_name}: {e}")
            return False

class DataSerializer:
    """Advanced data serialization utilities"""
    
    @staticmethod
    def compress_data(data: Any) -> bytes:
        """Compress data using gzip"""
        try:
            serialized = pickle.dumps(data)
            compressed = gzip.compress(serialized)
            return compressed
        except Exception as e:
            logger.error(f"Data compression failed: {e}")
            raise
    
    @staticmethod
    def decompress_data(compressed_data: bytes) -> Any:
        """Decompress gzip compressed data"""
        try:
            decompressed = gzip.decompress(compressed_data)
            data = pickle.loads(decompressed)
            return data
        except Exception as e:
            logger.error(f"Data decompression failed: {e}")
            raise
    
    @staticmethod
    def export_to_csv(data: List[Dict[str, Any]], file_path: Union[str, Path]) -> bool:
        """Export data to CSV file"""
        try:
            if not data:
                return False
            
            with open(file_path, 'w', newline='', encoding='utf-8') as f:
                fieldnames = data[0].keys()
                writer = csv.DictWriter(f, fieldnames=fieldnames)
                writer.writeheader()
                writer.writerows(data)
            
            logger.debug(f"Data exported to CSV: {file_path}")
            return True
        except Exception as e:
            logger.error(f"CSV export failed: {e}")
            return False
    
    @staticmethod
    def create_backup(file_path: Union[str, Path], backup_dir: str = "backups") -> bool:
        """Create a backup of a file"""
        try:
            source_path = Path(file_path)
            backup_dir_path = Path(backup_dir)
            backup_dir_path.mkdir(exist_ok=True)
            
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            backup_name = f"{source_path.stem}_{timestamp}{source_path.suffix}"
            backup_path = backup_dir_path / backup_name
            
            # Copy file
            import shutil
            shutil.copy2(source_path, backup_path)
            
            logger.info(f"Backup created: {backup_path}")
            return True
        except Exception as e:
            logger.error(f"Backup creation failed: {e}")
            return False