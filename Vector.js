class Vector {
    constructor(x1, y1, x2, y2) {
        this.x1 = x1;
        this.y1 = y1;
        this.x2 = x2;
        this.y2 = y2;
        this.a = x2 - x1;
        this.b = y2 - y1;
    }

    dot(other) {
        return this.a * other.a + this.b * other.b;
    }

    projectPoint(x, y) {
        const vectorToPoint = new Vector(this.x1, this.y1, x, y);
        const scalar = vectorToPoint.dot(this) / this.dot(this);
        return {
            x: this.x1 + scalar * this.a,
            y: this.y1 + scalar * this.b
        };
    }
}
