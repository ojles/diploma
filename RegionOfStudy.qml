import QtQuick 2.0

MapRegion {
    function hasInside(coordinate) {
        if (path.length < 3) {
            return false;
        }

        // Create a point for line segment from coordinate to infinite
        let extreme = {
            latitude: Number.MAX_VALUE,
            longitude: coordinate.longitude
        };

        // Count intersections of the above line with sides of path
        let count = 0;
        let i = 0;

        do {
            let next = (i + 1) % path.length;

            // Check if the line segment from 'coordinate' to 'extreme' intersects
            // with the line segment from 'path[i]' to 'path[next]'
            if (_doIntersect(path[i], path[next], coordinate, extreme))
            {
                // If the point 'coordinate' is colinear with line segment 'i-next',
                // then check if it lies on segment. If it lies, return true,
                // otherwise false
                if (_orientation(path[i], coordinate, path[next]) === 0) {
                   return _onSegment(path[i], coordinate, path[next]);
                }
                count++;
            }
            i = next;
        } while (i !== 0);

        return (count % 2) === 1;
    }

    function _onSegment(p, q, r) {
        return q.latitude <= Math.max(p.latitude, r.latitude)
                && q.latitude >= Math.min(p.latitude, r.latitude)
                && q.longitude <= Math.max(p.longitude, r.longitude)
                && q.longitude >= Math.min(p.longitude, r.longitude);
    }

    function _orientation(p, q, r)  {
        let orientation = (q.longitude - p.longitude) * (r.latitude - q.latitude)
                          - (q.latitude - p.latitude) * (r.longitude - q.longitude);
        if (orientation === 0) {
            return 0;
        }
        return orientation > 0 ? 1 : 2;
    }

    function _doIntersect(p1, q1, p2, q2)  {
        let o1 = _orientation(p1, q1, p2);
        let o2 = _orientation(p1, q1, q2);
        let o3 = _orientation(p2, q2, p1);
        let o4 = _orientation(p2, q2, q1);

        // General case
        if (o1 !== o2 && o3 !== o4) {
            return true;
        }

        // Special Cases
        // p1, q1 and p2 are colinear and p2 lies on segment p1q1
        if (o1 === 0 && _onSegment(p1, p2, q1)){
            return true;
        }

        // p1, q1 and p2 are colinear and q2 lies on segment p1q1
        if (o2 === 0 && _onSegment(p1, q2, q1)){
            return true;
        }

        // p2, q2 and p1 are colinear and p1 lies on segment p2q2
        if (o3 === 0 && _onSegment(p2, p1, q2)){
            return true;
        }

        // p2, q2 and q1 are colinear and q1 lies on segment p2q2
        if (o4 === 0 && _onSegment(p2, q1, q2)){
            return true;
        }

        // Doesn't fall in any of the above cases
        return false;
    }
}
