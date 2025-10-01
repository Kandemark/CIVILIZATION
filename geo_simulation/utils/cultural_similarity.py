"""Cultural similarity"""

# Module implementation

# utils/cultural_similarity.py
from typing import Dict, List, Optional, Set, Tuple
from dataclasses import dataclass, field
import numpy as np
import logging

logger = logging.getLogger(__name__)


@dataclass
class CulturalTraits:
    """Pure cultural traits for similarity calculation."""
    culture_id: str
    
    # Core cultural dimensions
    language_family: str
    language_features: Dict[str, float] = field(default_factory=dict)
    
    religion: str
    religious_practices: Set[str] = field(default_factory=set)
    
    social_structure: str
    customs: Set[str] = field(default_factory=set)
    values: Dict[str, float] = field(default_factory=dict)
    
    # Historical cultural relations
    historical_relations: Dict[str, float] = field(default_factory=dict)  # culture_id -> relation_score


class CulturalSimilarity:
    """Pure cultural similarity calculator without evolution or other dynamics."""
    
    def __init__(self):
        # Pre-defined cultural similarity matrices
        self.language_similarity = self._build_language_similarity_matrix()
        self.religion_similarity = self._build_religion_similarity_matrix()
        self.social_structure_similarity = self._build_social_structure_similarity()
        
    def _build_language_similarity_matrix(self) -> Dict[Tuple[str, str], float]:
        """Build language family similarity matrix."""
        families = [
            "indo_european", "sino_tibetan", "afro_asiatic", "niger_congo",
            "austronesian", "dravidian", "turkic", "uralic", "japonic"
        ]
        
        similarity = {}
        for i, fam1 in enumerate(families):
            for j, fam2 in enumerate(families):
                if fam1 == fam2:
                    similarity[(fam1, fam2)] = 1.0
                else:
                    # Linguistic proximity scores
                    proximity_scores = {
                        ("indo_european", "uralic"): 0.3,
                        ("turkic", "uralic"): 0.2,
                        ("sino_tibetan", "austronesian"): 0.1,
                    }
                    key = (fam1, fam2)
                    reverse_key = (fam2, fam1)
                    similarity[key] = proximity_scores.get(key, proximity_scores.get(reverse_key, 0.1))
        
        return similarity
    
    def _build_religion_similarity_matrix(self) -> Dict[Tuple[str, str], float]:
        """Build religion similarity matrix."""
        religions = [
            "animism", "shamanism", "polytheism", "monotheism", 
            "buddhism", "hinduism", "taoism", "confucianism"
        ]
        
        similarity = {}
        for rel1 in religions:
            for rel2 in religions:
                if rel1 == rel2:
                    similarity[(rel1, rel2)] = 1.0
                else:
                    # Religious family groupings
                    abrahamic = {"monotheism"}
                    dharmic = {"hinduism", "buddhism"}
                    taoic = {"taoism", "confucianism"}
                    traditional = {"animism", "shamanism", "polytheism"}
                    
                    if (rel1 in abrahamic and rel2 in abrahamic) or \
                       (rel1 in dharmic and rel2 in dharmic) or \
                       (rel1 in taoic and rel2 in taoic) or \
                       (rel1 in traditional and rel2 in traditional):
                        similarity[(rel1, rel2)] = 0.6
                    else:
                        similarity[(rel1, rel2)] = 0.2
        
        return similarity
    
    def _build_social_structure_similarity(self) -> Dict[Tuple[str, str], float]:
        """Build social structure similarity matrix."""
        structures = [
            "egalitarian", "kinship_based", "tribal", "hierarchical",
            "feudal", "caste_based", "bureaucratic"
        ]
        
        similarity = {}
        for struct1 in structures:
            for struct2 in structures:
                if struct1 == struct2:
                    similarity[(struct1, struct2)] = 1.0
                else:
                    # Group similar structures
                    simple_structures = {"egalitarian", "kinship_based"}
                    intermediate_structures = {"tribal", "hierarchical"}
                    complex_structures = {"feudal", "caste_based", "bureaucratic"}
                    
                    if (struct1 in simple_structures and struct2 in simple_structures) or \
                       (struct1 in intermediate_structures and struct2 in intermediate_structures) or \
                       (struct1 in complex_structures and struct2 in complex_structures):
                        similarity[(struct1, struct2)] = 0.7
                    else:
                        similarity[(struct1, struct2)] = 0.3
        
        return similarity
    
    def calculate_similarity(self, culture_a: CulturalTraits, culture_b: CulturalTraits) -> float:
        """
        Calculate overall cultural similarity between two cultures.
        Returns a score between 0.0 (completely different) and 1.0 (identical).
        """
        similarities = []
        weights = []
        
        # 1. Language similarity (weight: 0.3)
        lang_sim = self._calculate_language_similarity(culture_a, culture_b)
        similarities.append(lang_sim)
        weights.append(0.3)
        
        # 2. Religion similarity (weight: 0.25)
        rel_sim = self._calculate_religion_similarity(culture_a, culture_b)
        similarities.append(rel_sim)
        weights.append(0.25)
        
        # 3. Social structure similarity (weight: 0.2)
        social_sim = self._calculate_social_structure_similarity(culture_a, culture_b)
        similarities.append(social_sim)
        weights.append(0.2)
        
        # 4. Customs similarity (weight: 0.15)
        customs_sim = self._calculate_customs_similarity(culture_a, culture_b)
        similarities.append(customs_sim)
        weights.append(0.15)
        
        # 5. Values similarity (weight: 0.1)
        values_sim = self._calculate_values_similarity(culture_a, culture_b)
        similarities.append(values_sim)
        weights.append(0.1)
        
        # Calculate weighted average
        total_similarity = np.average(similarities, weights=weights)
        
        # Apply historical relations modifier if available
        historical_modifier = self._get_historical_modifier(culture_a, culture_b)
        adjusted_similarity = total_similarity * historical_modifier
        
        return float(np.clip(adjusted_similarity, 0.0, 1.0))
    
    def _calculate_language_similarity(self, culture_a: CulturalTraits, culture_b: CulturalTraits) -> float:
        """Calculate language similarity between two cultures."""
        # Family-level similarity
        family_sim = self.language_similarity.get(
            (culture_a.language_family, culture_b.language_family), 0.1
        )
        
        # Feature-based similarity (if features are provided)
        feature_sim = self._calculate_feature_similarity(
            culture_a.language_features, 
            culture_b.language_features
        )
        
        # Combine family and feature similarity
        return 0.7 * family_sim + 0.3 * feature_sim
    
    def _calculate_religion_similarity(self, culture_a: CulturalTraits, culture_b: CulturalTraits) -> float:
        """Calculate religious similarity between two cultures."""
        # Core religion similarity
        religion_sim = self.religion_similarity.get(
            (culture_a.religion, culture_b.religion), 0.3
        )
        
        # Practices similarity (Jaccard index)
        practices_sim = self._calculate_set_similarity(
            culture_a.religious_practices, 
            culture_b.religious_practices
        )
        
        return 0.6 * religion_sim + 0.4 * practices_sim
    
    def _calculate_social_structure_similarity(self, culture_a: CulturalTraits, culture_b: CulturalTraits) -> float:
        """Calculate social structure similarity."""
        return self.social_structure_similarity.get(
            (culture_a.social_structure, culture_b.social_structure), 0.5
        )
    
    def _calculate_customs_similarity(self, culture_a: CulturalTraits, culture_b: CulturalTraits) -> float:
        """Calculate customs similarity using Jaccard index."""
        return self._calculate_set_similarity(culture_a.customs, culture_b.customs)
    
    def _calculate_values_similarity(self, culture_a: CulturalTraits, culture_b: CulturalTraits) -> float:
        """Calculate cultural values similarity using cosine similarity."""
        if not culture_a.values or not culture_b.values:
            return 0.5
        
        # Get all value keys
        all_keys = set(culture_a.values.keys()) | set(culture_b.values.keys())
        if not all_keys:
            return 0.5
        
        # Create vectors
        vec_a = [culture_a.values.get(key, 0.5) for key in all_keys]
        vec_b = [culture_b.values.get(key, 0.5) for key in all_keys]
        
        # Calculate cosine similarity
        dot_product = np.dot(vec_a, vec_b)
        norm_a = np.linalg.norm(vec_a)
        norm_b = np.linalg.norm(vec_b)
        
        if norm_a == 0 or norm_b == 0:
            return 0.5
        
        return dot_product / (norm_a * norm_b)
    
    def _calculate_feature_similarity(self, features_a: Dict[str, float], features_b: Dict[str, float]) -> float:
        """Calculate similarity between feature dictionaries."""
        if not features_a or not features_b:
            return 0.5
        
        common_features = set(features_a.keys()) & set(features_b.keys())
        if not common_features:
            return 0.3
        
        # Calculate average similarity for common features
        similarities = []
        for feature in common_features:
            diff = abs(features_a[feature] - features_b[feature])
            similarities.append(1.0 - diff)
        
        return np.mean(similarities) if similarities else 0.3
    
    def _calculate_set_similarity(self, set_a: Set[str], set_b: Set[str]) -> float:
        """Calculate Jaccard similarity between two sets."""
        if not set_a and not set_b:
            return 0.5
        if not set_a or not set_b:
            return 0.2
        
        intersection = len(set_a & set_b)
        union = len(set_a | set_b)
        
        return intersection / union if union > 0 else 0.0
    
    def _get_historical_modifier(self, culture_a: CulturalTraits, culture_b: CulturalTraits) -> float:
        """Get historical relations modifier for similarity calculation."""
        # Check if cultures have historical relations recorded
        relation_ab = culture_a.historical_relations.get(culture_b.culture_id)
        relation_ba = culture_b.historical_relations.get(culture_a.culture_id)
        
        if relation_ab is not None and relation_ba is not None:
            # Average both relations
            avg_relation = (relation_ab + relation_ba) / 2
            # Convert from -1..1 scale to 0.8..1.2 modifier scale
            return 1.0 + avg_relation * 0.2
        elif relation_ab is not None:
            return 1.0 + relation_ab * 0.2
        elif relation_ba is not None:
            return 1.0 + relation_ba * 0.2
        
        # No historical relations - neutral modifier
        return 1.0
    
    def get_similarity_breakdown(self, culture_a: CulturalTraits, culture_b: CulturalTraits) -> Dict[str, float]:
        """Get detailed breakdown of cultural similarity components."""
        return {
            "language_similarity": self._calculate_language_similarity(culture_a, culture_b),
            "religion_similarity": self._calculate_religion_similarity(culture_a, culture_b),
            "social_structure_similarity": self._calculate_social_structure_similarity(culture_a, culture_b),
            "customs_similarity": self._calculate_customs_similarity(culture_a, culture_b),
            "values_similarity": self._calculate_values_similarity(culture_a, culture_b),
            "overall_similarity": self.calculate_similarity(culture_a, culture_b)
        }
    
    def find_most_similar_culture(self, target_culture: CulturalTraits, 
                                candidate_cultures: List[CulturalTraits]) -> Tuple[Optional[CulturalTraits], float]:
        """Find the most similar culture from a list of candidates."""
        if not candidate_cultures:
            return None, 0.0
        
        best_similarity = -1.0
        best_culture = None
        
        for candidate in candidate_cultures:
            if candidate.culture_id == target_culture.culture_id:
                continue
            
            similarity = self.calculate_similarity(target_culture, candidate)
            if similarity > best_similarity:
                best_similarity = similarity
                best_culture = candidate
        
        return best_culture, best_similarity
    
    def get_similarity_matrix(self, cultures: List[CulturalTraits]) -> np.ndarray:
        """Create a similarity matrix for all cultures."""
        n = len(cultures)
        matrix = np.zeros((n, n))
        
        for i in range(n):
            for j in range(n):
                if i == j:
                    matrix[i][j] = 1.0  # Self-similarity is always 1.0
                else:
                    matrix[i][j] = self.calculate_similarity(cultures[i], cultures[j])
        
        return matrix