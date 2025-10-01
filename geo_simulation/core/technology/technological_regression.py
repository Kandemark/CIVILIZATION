"""Loss of knowledge and skills"""

# Module implementation

"""
Technological Regression Module

Models and analyzes scenarios of technological regression, decline,
and collapse, including their causes, patterns, and recovery processes.
"""

from typing import Dict, List, Optional, Set, Tuple
from dataclasses import dataclass
from enum import Enum
from datetime import datetime, timedelta
import math
import statistics


class RegressionCause(Enum):
    """Causes of technological regression"""
    RESOURCE_DEPLETION = "resource_depletion"
    ENVIRONMENTAL_COLLAPSE = "environmental_collapse"
    SOCIETAL_COLLAPSE = "societal_collapse"
    KNOWLEDGE_LOSS = "knowledge_loss"
    ECONOMIC_COLLAPSE = "economic_collapse"
    CONFLICT = "conflict"
    PANDEMIC = "pandemic"
    INFRASTRUCTURE_FAILURE = "infrastructure_failure"
    REGULATORY_OVERREACH = "regulatory_overreach"


class RegressionPattern(Enum):
    """Patterns of technological regression"""
    GRADUAL_DECLINE = "gradual_decline"
    CASCADE_FAILURE = "cascade_failure"
    SUDDEN_COLLAPSE = "sudden_collapse"
    SELECTIVE_LOSS = "selective_loss"
    KNOWLEDGE_FRAGMENTATION = "knowledge_fragmentation"
    INFRASTRUCTURE_EROSION = "infrastructure_erosion"


class RecoveryPotential(Enum):
    """Potential for technological recovery"""
    LOW = "low"
    MEDIUM = "medium"
    HIGH = "high"
    VERY_HIGH = "very_high"


@dataclass
class RegressionEvent:
    """Represents a technological regression event"""
    
    id: str
    name: str
    description: str
    causes: Set[RegressionCause]
    pattern: RegressionPattern
    
    # Temporal properties
    start_date: datetime
    duration_days: int  # Duration of active regression phase
    
    # Impact metrics (0.0 to 1.0)
    technological_impact: float
    economic_impact: float
    social_impact: float
    knowledge_loss: float
    infrastructure_loss: float
    
    # Recovery properties
    recovery_potential: RecoveryPotential
    recovery_time_estimate: int  # Days to full recovery
    
    # Affected domains
    affected_technologies: Set[str]
    affected_infrastructure: Set[str]
    lost_knowledge: Set[str]
    
    def __post_init__(self):
        """Validate regression event data"""
        assert self.duration_days > 0, "Duration must be positive"
        assert 0.0 <= self.technological_impact <= 1.0, "Technological impact must be between 0 and 1"
        assert 0.0 <= self.economic_impact <= 1.0, "Economic impact must be between 0 and 1"
        assert 0.0 <= self.social_impact <= 1.0, "Social impact must be between 0 and 1"
        assert 0.0 <= self.knowledge_loss <= 1.0, "Knowledge loss must be between 0 and 1"
        assert 0.0 <= self.infrastructure_loss <= 1.0, "Infrastructure loss must be between 0 and 1"
        assert self.recovery_time_estimate > 0, "Recovery time must be positive"
    
    @property
    def end_date(self) -> datetime:
        """Calculate end date of regression event"""
        return self.start_date + timedelta(days=self.duration_days)
    
    @property
    def overall_severity(self) -> float:
        """Calculate overall severity of regression event"""
        weights = {
            'technological': 0.25,
            'economic': 0.20,
            'social': 0.20,
            'knowledge': 0.20,
            'infrastructure': 0.15
        }
        
        severity = (
            weights['technological'] * self.technological_impact +
            weights['economic'] * self.economic_impact +
            weights['social'] * self.social_impact +
            weights['knowledge'] * self.knowledge_loss +
            weights['infrastructure'] * self.infrastructure_loss
        )
        
        return round(severity, 4)
    
    @property
    def is_ongoing(self) -> bool:
        """Check if regression event is currently ongoing"""
        now = datetime.now()
        return self.start_date <= now <= self.end_date
    
    @property
    def recovery_difficulty(self) -> float:
        """Calculate difficulty of recovery"""
        recovery_factors = {
            RecoveryPotential.LOW: 0.9,
            RecoveryPotential.MEDIUM: 0.7,
            RecoveryPotential.HIGH: 0.4,
            RecoveryPotential.VERY_HIGH: 0.2
        }
        
        base_difficulty = recovery_factors.get(self.recovery_potential, 0.5)
        
        # Adjust for severity and knowledge loss
        adjusted_difficulty = base_difficulty * (1 + self.overall_severity * 0.5)
        
        return min(1.0, adjusted_difficulty)
    
    def get_critical_dependencies(self) -> List[str]:
        """Identify critical technologies and infrastructure for recovery"""
        critical_items = []
        
        # Technologies with high impact and low recovery potential are critical
        if self.technological_impact > 0.7 and self.recovery_potential in [RecoveryPotential.LOW, RecoveryPotential.MEDIUM]:
            critical_items.extend(self.affected_technologies)
        
        # Infrastructure is always critical if significantly affected
        if self.infrastructure_loss > 0.5:
            critical_items.extend(self.affected_infrastructure)
        
        return list(set(critical_items))


class RegressionAnalyzer:
    """Analyzes technological regression events and patterns"""
    
    def __init__(self):
        self.regression_events: Dict[str, RegressionEvent] = {}
        self.technology_baseline: Dict[str, float] = {}  # Technology -> capability level
    
    def add_regression_event(self, event: RegressionEvent) -> bool:
        """Add a regression event to the analyzer"""
        if event.id in self.regression_events:
            return False
        
        self.regression_events[event.id] = event
        return True
    
    def remove_regression_event(self, event_id: str) -> bool:
        """Remove a regression event"""
        if event_id not in self.regression_events:
            return False
        
        del self.regression_events[event_id]
        return True
    
    def set_technology_baseline(self, baseline: Dict[str, float]):
        """Set the baseline technology capabilities before regression"""
        self.technology_baseline = baseline.copy()
    
    def calculate_regression_metrics(self) -> Dict[str, float]:
        """Calculate comprehensive regression metrics"""
        if not self.regression_events:
            return {}
        
        events = list(self.regression_events.values())
        
        metrics = {
            'total_events': len(events),
            'average_severity': statistics.mean(event.overall_severity for event in events),
            'average_duration_days': statistics.mean(event.duration_days for event in events),
            'recovery_difficulty_index': statistics.mean(event.recovery_difficulty for event in events),
            'cascade_risk': self._calculate_cascade_risk(),
            'systemic_vulnerability': self._calculate_systemic_vulnerability(),
        }
        
        # Cause frequency analysis
        cause_frequency = {}
        for cause in RegressionCause:
            cause_count = sum(1 for event in events if cause in event.causes)
            cause_frequency[cause.value] = cause_count / len(events)
        metrics['cause_frequency'] = cause_frequency
        
        return metrics
    
    def _calculate_cascade_risk(self) -> float:
        """Calculate risk of regression events cascading across systems"""
        events = list(self.regression_events.values())
        if not events:
            return 0.0
        
        # Cascade risk increases with event severity and interconnectedness
        severity_scores = [event.overall_severity for event in events]
        avg_severity = statistics.mean(severity_scores)
        
        # Estimate interconnectedness based on affected domains overlap
        interconnectedness = 0.0
        for i, event1 in enumerate(events):
            for event2 in events[i+1:]:
                overlap = len(event1.affected_technologies.intersection(event2.affected_technologies))
                max_domains = max(len(event1.affected_technologies), len(event2.affected_technologies), 1)
                interconnectedness += overlap / max_domains
        
        total_pairs = len(events) * (len(events) - 1) / 2
        avg_interconnectedness = interconnectedness / total_pairs if total_pairs > 0 else 0.0
        
        cascade_risk = avg_severity * avg_interconnectedness
        return min(1.0, cascade_risk)
    
    def _calculate_systemic_vulnerability(self) -> float:
        """Calculate overall systemic vulnerability to regression"""
        events = list(self.regression_events.values())
        if not events:
            return 0.0
        
        # Vulnerability based on frequency, severity, and recovery difficulty
        frequency_factor = min(1.0, len(events) / 10)  # Normalize by assuming 10 events is maximum
        avg_severity = statistics.mean(event.overall_severity for event in events)
        avg_recovery_difficulty = statistics.mean(event.recovery_difficulty for event in events)
        
        vulnerability = (frequency_factor + avg_severity + avg_recovery_difficulty) / 3
        return vulnerability
    
    def assess_recovery_progress(self, current_capabilities: Dict[str, float]) -> Dict[str, float]:
        """
        Assess recovery progress compared to baseline capabilities.
        
        Args:
            current_capabilities: Current technology capability levels
            
        Returns:
            Recovery progress metrics
        """
        if not self.technology_baseline:
            return {}
        
        progress_metrics = {}
        
        # Calculate recovery for each technology
        recovery_scores = []
        for tech, baseline_capability in self.technology_baseline.items():
            current_capability = current_capabilities.get(tech, 0.0)
            recovery_ratio = current_capability / baseline_capability if baseline_capability > 0 else 1.0
            recovery_scores.append(min(1.0, recovery_ratio))
        
        if recovery_scores:
            progress_metrics['average_recovery'] = statistics.mean(recovery_scores)
            progress_metrics['full_recovery_count'] = sum(1 for score in recovery_scores if score >= 0.95)
            progress_metrics['min_recovery'] = min(recovery_scores) if recovery_scores else 0.0
            progress_metrics['max_recovery'] = max(recovery_scores) if recovery_scores else 0.0
        
        # Overall recovery assessment
        if progress_metrics:
            avg_recovery = progress_metrics['average_recovery']
            if avg_recovery >= 0.95:
                progress_metrics['recovery_status'] = 'complete'
            elif avg_recovery >= 0.7:
                progress_metrics['recovery_status'] = 'advanced'
            elif avg_recovery >= 0.4:
                progress_metrics['recovery_status'] = 'moderate'
            elif avg_recovery >= 0.1:
                progress_metrics['recovery_status'] = 'early'
            else:
                progress_metrics['recovery_status'] = 'minimal'
        
        return progress_metrics
    
    def predict_future_regression_risk(self, 
                                    system_indicators: Dict[str, float],
                                    time_horizon: int = 365) -> Dict[str, float]:
        """
        Predict future regression risk based on system indicators.
        
        Args:
            system_indicators: Current system health indicators
            time_horizon: Prediction horizon in days
            
        Returns:
            Regression risk predictions
        """
        risk_factors = {}
        
        # Analyze each indicator for risk contribution
        indicator_weights = {
            'resource_scarcity': 0.15,
            'social_stability': 0.20,
            'economic_health': 0.20,
            'infrastructure_age': 0.15,
            'knowledge_preservation': 0.15,
            'environmental_stress': 0.15
        }
        
        total_risk = 0.0
        for indicator, weight in indicator_weights.items():
            indicator_value = system_indicators.get(indicator, 0.5)
            # Higher values indicate higher risk for most indicators
            # Except for social_stability, economic_health, knowledge_preservation where lower values indicate higher risk
            if indicator in ['social_stability', 'economic_health', 'knowledge_preservation']:
                risk_contribution = (1 - indicator_value) * weight
            else:
                risk_contribution = indicator_value * weight
            
            risk_factors[indicator] = risk_contribution
            total_risk += risk_contribution
        
        risk_factors['total_risk'] = min(1.0, total_risk)
        
        # Adjust for time horizon (longer horizon = higher risk)
        time_adjustment = min(1.0, time_horizon / 1825)  # 5 years maximum
        risk_factors['time_adjusted_risk'] = min(1.0, total_risk * (1 + time_adjustment * 0.5))
        
        # Identify most likely causes based on risk factors
        likely_causes = self._identify_likely_causes(risk_factors)
        risk_factors['likely_causes'] = likely_causes
        
        return risk_factors
    
    def _identify_likely_causes(self, risk_factors: Dict[str, float]) -> List[Tuple[str, float]]:
        """Identify most likely regression causes based on risk factors"""
        cause_mappings = {
            'resource_scarcity': RegressionCause.RESOURCE_DEPLETION,
            'environmental_stress': RegressionCause.ENVIRONMENTAL_COLLAPSE,
            'social_stability': RegressionCause.SOCIETAL_COLLAPSE,
            'knowledge_preservation': RegressionCause.KNOWLEDGE_LOSS,
            'economic_health': RegressionCause.ECONOMIC_COLLAPSE,
            'infrastructure_age': RegressionCause.INFRASTRUCTURE_FAILURE,
        }
        
        likely_causes = []
        for factor, risk_value in risk_factors.items():
            if factor in cause_mappings and risk_value > 0.3:
                cause = cause_mappings[factor]
                likely_causes.append((cause.value, risk_value))
        
        # Sort by risk value
        likely_causes.sort(key=lambda x: x[1], reverse=True)
        return likely_causes
    
    def generate_resilience_recommendations(self, 
                                         risk_assessment: Dict[str, float]) -> List[Dict]:
        """
        Generate recommendations for improving technological resilience.
        
        Args:
            risk_assessment: Results from risk prediction
            
        Returns:
            List of resilience recommendations
        """
        recommendations = []
        
        # Analyze high-risk areas and generate targeted recommendations
        high_risk_threshold = 0.6
        
        for risk_factor, risk_value in risk_assessment.items():
            if risk_value >= high_risk_threshold and risk_factor != 'total_risk':
                recommendation = self._create_resilience_recommendation(risk_factor, risk_value)
                if recommendation:
                    recommendations.append(recommendation)
        
        # General resilience recommendations
        if risk_assessment.get('total_risk', 0) > 0.7:
            recommendations.append({
                'category': 'systemic',
                'priority': 'critical',
                'recommendation': 'Implement comprehensive system-wide resilience strategy',
                'actions': [
                    'Diversify critical technology dependencies',
                    'Establish redundant infrastructure systems',
                    'Create knowledge preservation protocols',
                    'Develop rapid response recovery plans'
                ]
            })
        
        # Sort by priority
        priority_order = {'critical': 0, 'high': 1, 'medium': 2, 'low': 3}
        recommendations.sort(key=lambda x: priority_order.get(x['priority'], 4))
        
        return recommendations
    
    def _create_resilience_recommendation(self, risk_factor: str, risk_value: float) -> Dict:
        """Create specific resilience recommendation for a risk factor"""
        recommendation_templates = {
            'resource_scarcity': {
                'category': 'resources',
                'priority': 'high' if risk_value > 0.7 else 'medium',
                'recommendation': 'Develop resource diversification and conservation strategies',
                'actions': [
                    'Identify alternative resource sources',
                    'Implement recycling and reuse systems',
                    'Develop resource-efficient technologies'
                ]
            },
            'social_stability': {
                'category': 'social',
                'priority': 'critical' if risk_value > 0.8 else 'high',
                'recommendation': 'Strengthen social cohesion and crisis response systems',
                'actions': [
                    'Enhance community resilience networks',
                    'Develop social safety nets',
                    'Establish conflict resolution mechanisms'
                ]
            },
            'economic_health': {
                'category': 'economic',
                'priority': 'high',
                'recommendation': 'Build economic resilience and diversification',
                'actions': [
                    'Diversify economic base',
                    'Establish emergency funds',
                    'Develop adaptive economic policies'
                ]
            },
            'infrastructure_age': {
                'category': 'infrastructure',
                'priority': 'high' if risk_value > 0.6 else 'medium',
                'recommendation': 'Modernize and reinforce critical infrastructure',
                'actions': [
                    'Accelerate infrastructure upgrades',
                    'Implement redundancy systems',
                    'Develop rapid repair capabilities'
                ]
            },
            'knowledge_preservation': {
                'category': 'knowledge',
                'priority': 'medium',
                'recommendation': 'Enhance knowledge preservation and transmission',
                'actions': [
                    'Create distributed knowledge repositories',
                    'Establish mentorship programs',
                    'Document critical processes and skills'
                ]
            },
            'environmental_stress': {
                'category': 'environmental',
                'priority': 'high' if risk_value > 0.7 else 'medium',
                'recommendation': 'Implement environmental protection and adaptation measures',
                'actions': [
                    'Develop climate resilience strategies',
                    'Protect critical ecosystems',
                    'Implement sustainable resource management'
                ]
            }
        }
        
        return recommendation_templates.get(risk_factor, {})