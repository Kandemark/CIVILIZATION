"""Language development and diffusion"""

# Module implementation

from typing import Dict, List, Set, Optional, Tuple
from dataclasses import dataclass
import random
import math

@dataclass 
class InfinitePhonology:
    """Phonology that can evolve without limits"""
    core_phonemes: Set[str]
    extended_phonemes: Set[str]
    exotic_phonemes: Set[str]  # For advanced languages
    phoneme_complexity: float = 1.0
    
    def add_phoneme(self, phoneme: str, complexity_boost: float = 0.1):
        """Add new phoneme, increasing complexity"""
        if phoneme not in self.core_phonemes and phoneme not in self.extended_phonemes:
            if random.random() < 0.7:
                self.extended_phonemes.add(phoneme)
            else:
                self.exotic_phonemes.add(phoneme)
            
            self.phoneme_complexity += complexity_boost

class InfiniteLanguage:
    """A language that can evolve infinitely without repetition"""
    
    def __init__(self, language_id: str, parent_culture_id: str):
        self.id = language_id
        self.parent_culture_id = parent_culture_id
        self.name = self._generate_infinite_name()
        self.phonology = self._generate_base_phonology()
        self.vocabulary: Dict[str, str] = {}
        self.grammatical_complexity = 1.0
        self.expressiveness = 1.0
        self.conceptual_depth = 1.0
        
        # Infinite scaling
        self.language_age = 0
        self.evolution_index = 0.0
        self.lexical_richness = 1.0
        
        # Generate initial vocabulary
        self._generate_infinite_vocabulary()
    
    def _generate_infinite_name(self) -> str:
        """Generate unique language names without repetition"""
        # Use procedural generation that can create infinite variations
        syllable_patterns = ["CV", "CVC", "VC", "CVV", "CCVC", "CVCC"]
        pattern = random.choice(syllable_patterns)
        
        name = ""
        for char in pattern:
            if char == 'C':
                name += random.choice('ptkbdgmnszʃʒʧʤ')
            elif char == 'V':
                name += random.choice('aeiouæøy')
            else:  # Double or complex
                name += random.choice('aeiou')
        
        return name.capitalize()
    
    def _generate_base_phonology(self) -> InfinitePhonology:
        """Generate base phonology that can expand infinitely"""
        base_vowels = set('aeiou')
        base_consonants = set('ptkbdgmnsz')
        
        return InfinitePhonology(
            core_phonemes=base_vowels | base_consonants,
            extended_phonemes=set(),
            exotic_phonemes=set()
        )
    
    def _generate_infinite_vocabulary(self):
        """Generate initial vocabulary that can expand without limits"""
        core_concepts = ["existence", "relation", "process", "quality", "quantity"]
        
        for concept in core_concepts:
            self.vocabulary[concept] = self._generate_unique_word(concept)
    
    def _generate_unique_word(self, concept: str) -> str:
        """Generate words that are unique and can evolve"""
        # Start with basic structure
        base_structure = random.choice(["CVC", "CVCCV", "CVCVC", "VCCVC"])
        word = ""
        
        for char in base_structure:
            if char == 'C':
                # Choose from available phonemes weighted by complexity
                all_cons = (list(self.phonology.core_phonemes) + 
                           list(self.phonology.extended_phonemes) + 
                           list(self.phonology.exotic_phonemes))
                cons = [c for c in all_cons if c not in 'aeiou']
                if cons:
                    word += random.choice(cons)
            else:  # 'V'
                all_vowels = [c for c in self.phonology.core_phonemes if c in 'aeiou']
                if all_vowels:
                    word += random.choice(all_vowels)
        
        return word
    
    def evolve(self, cultural_complexity: float, contact_influences: List[float]):
        """Evolve language infinitely based on cultural development"""
        self.language_age += 1
        
        # Language evolution scales with cultural complexity
        evolution_rate = 0.01 * cultural_complexity
        self.evolution_index += evolution_rate
        
        # Phonological expansion
        self._expand_phonology()
        
        # Vocabulary growth (infinite)
        self._grow_vocabulary()
        
        # Grammatical evolution
        self._evolve_grammar()
        
        # Expressiveness development
        self._develop_expressiveness()
    
    def _expand_phonology(self):
        """Expand phonology without limits"""
        expansion_chance = 0.05 * (1.0 + self.evolution_index * 0.1)
        
        if random.random() < expansion_chance:
            # Generate new phoneme
            phoneme_types = [
                ("vowel", 'iyɨʉɯuɪʏʊeøɘɵɤoəɛœɜɞʌɔæɐaɶɑɒ'),
                ("consonant", 'pbtdʈɖcɟkɡqɢʔɴŋɲɳnɱmʙrʀⱱɾɽɸβfvθðszʃʒʂʐçʝxɣχʁħʕhɦɬɮʋɹɻjɰlɭʎʟ'),
                ("click", 'ʘǀǃǂǁ'),
                ("tone", '˥˦˧˨˩')
            ]
            
            phoneme_type, possibilities = random.choice(phoneme_types)
            new_phoneme = random.choice(possibilities)
            
            self.phonology.add_phoneme(new_phoneme)
    
    def _grow_vocabulary(self):
        """Grow vocabulary infinitely"""
        # Vocabulary growth scales with evolution
        growth_rate = 0.1 * (1.0 + self.evolution_index * 0.5)
        
        if random.random() < growth_rate:
            # Create new concepts
            concept_domains = [
                "abstract_thought", "scientific_concept", "emotional_nuance",
                "philosophical_idea", "technological_innovation", "artistic_expression"
            ]
            
            domain = random.choice(concept_domains)
            concept_id = f"{domain}_{random.randint(1, 10000)}"
            
            # Only add if concept doesn't exist (infinite space)
            if concept_id not in self.vocabulary:
                self.vocabulary[concept_id] = self._generate_unique_word(concept_id)
                self.lexical_richness += 0.1
    
    def _evolve_grammar(self):
        """Evolve grammatical complexity without upper bound"""
        # Grammar can become infinitely complex
        complexity_growth = 0.01 * (1.0 + self.evolution_index * 0.2)
        
        # Sometimes simplify, sometimes complexify
        if random.random() < 0.7:  # 70% chance to become more complex
            self.grammatical_complexity += complexity_growth
        else:  # 30% chance to simplify (natural language balance)
            self.grammatical_complexity = max(1.0, self.grammatical_complexity - complexity_growth * 0.5)
    
    def _develop_expressiveness(self):
        """Develop expressiveness without limits"""
        # Languages can become infinitely expressive
        expressiveness_growth = 0.02 * (1.0 + self.evolution_index * 0.3)
        self.expressiveness += expressiveness_growth
        
        # Conceptual depth increases with civilization development
        self.conceptual_depth += expressiveness_growth * 0.5
    
    def create_derived_language(self, isolation_factor: float) -> 'InfiniteLanguage':
        """Create a new language derived from this one"""
        derived_id = f"lang_derived_{self.id}_{random.randint(1000, 9999)}"
        derived_lang = InfiniteLanguage(derived_id, self.parent_culture_id)
        
        # Start with similar but diverged phonology
        derived_lang.phonology.core_phonemes = self.phonology.core_phonemes.copy()
        derived_lang.phonology.extended_phonemes = self.phonology.extended_phonemes.copy()
        
        # Apply divergence based on isolation
        divergence_amount = isolation_factor * 0.3
        
        # Phonological divergence
        if random.random() < divergence_amount:
            # Change some phonemes
            if derived_lang.phonology.core_phonemes:
                old_phoneme = random.choice(list(derived_lang.phonology.core_phonemes))
                derived_lang.phonology.core_phonemes.remove(old_phoneme)
                new_phoneme = self._generate_unique_phoneme()
                derived_lang.phonology.core_phonemes.add(new_phoneme)
        
        # Vocabulary divergence
        for concept, word in self.vocabulary.items():
            if random.random() < divergence_amount:
                # Alter the word
                derived_lang.vocabulary[concept] = self._alter_word(word)
            else:
                derived_lang.vocabulary[concept] = word
        
        return derived_lang
    
    def _generate_unique_phoneme(self) -> str:
        """Generate a unique phoneme"""
        phoneme_sources = 'iyɨʉɯuɪʏʊeøɘɵɤoəɛœɜɞʌɔæɐaɶɑɒpbtdʈɖcɟkɡqɢʔɴŋɲɳnɱmʙrʀⱱɾɽɸβfvθðszʃʒʂʐçʝxɣχʁħʕhɦɬɮʋɹɻjɰlɭʎʟʘǀǃǂǁ'
        return random.choice(phoneme_sources)
    
    def _alter_word(self, word: str) -> str:
        """Alter a word to create divergence"""
        alterations = [
            lambda w: w.replace('k', 'g'),
            lambda w: w.replace('p', 'b'), 
            lambda w: w + random.choice('aeiou'),
            lambda w: w[:-1] if len(w) > 2 else w,
            lambda w: w[1:] + w[0],  # Metathesis
            lambda w: w + 'ʰ',  # Aspiration
        ]
        
        altered = word
        for alteration in random.sample(alterations, random.randint(1, 2)):
            if random.random() < 0.6:
                altered = alteration(altered)
        
        return altered

class InfiniteLanguageEvolution:
    """Manages infinite language evolution without repetition"""
    
    def __init__(self):
        self.languages: Dict[str, InfiniteLanguage] = {}
        self.language_network: Dict[str, Set[str]] = {}  # Language relationships
        
    def create_primordial_language(self, culture_id: str) -> InfiniteLanguage:
        """Create a completely new primordial language"""
        lang_id = f"primordial_{len(self.languages)}"
        language = InfiniteLanguage(lang_id, culture_id)
        
        self.languages[lang_id] = language
        self.language_network[lang_id] = set()
        
        return language
    
    def evolve_all_languages(self, cultural_developments: Dict[str, float]):
        """Evolve all languages based on cultural progress"""
        for lang_id, language in self.languages.items():
            culture_complexity = cultural_developments.get(language.parent_culture_id, 1.0)
            
            # Get contact influences
            contact_strengths = []
            for contact_id in self.language_network.get(lang_id, set()):
                if contact_id in self.languages:
                    # Contact strength based on relationship
                    contact_strengths.append(0.3)  # Base contact influence
            
            language.evolve(culture_complexity, contact_strengths)
            
            # Check for language speciation
            self._check_language_speciation(language, culture_complexity)
    
    def _check_language_speciation(self, language: InfiniteLanguage, culture_complexity: float):
        """Check if language should speciate into new languages"""
        speciation_chance = 0.01 * culture_complexity
        
        if random.random() < speciation_chance and len(self.languages) < 1000:  # Reasonable limit
            isolation = random.uniform(0.2, 0.9)
            new_language = language.create_derived_language(isolation)
            
            self.languages[new_language.id] = new_language
            self.language_network[new_language.id] = {language.id}
            self.language_network[language.id].add(new_language.id)