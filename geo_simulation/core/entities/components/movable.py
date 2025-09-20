"""Movable component"""

# Module implementation

class Movable:
    def __init__(self, speed=1):
        self.speed = speed

    def move(self, dx, dy, position):
        position.x += dx * self.speed
        position.y += dy * self.speed
