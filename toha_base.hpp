#ifndef BASE_HPP_INCLUDED
#define BASE_HPP_INCLUDED

#define EPSILON 0.000001

class Point3 {

public:
    double x,y,z;

    Point3(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Point3() {}

    Point3 operator+(Point3 pt) {
        Point3 temp(x + pt.x, y + pt.y, z + pt.z);
        return temp;
    }

    Point3 operator-(Point3 pt) {
        Point3 temp(x - pt.x, y - pt.y, z - pt.z);
        return temp;
    }

    Point3 operator*(double pt) {
        Point3 temp(x*pt, y*pt, z*pt);
        return temp;
    }

    Point3 operator/(double pt) {
        Point3 temp(x/pt, y/pt, z/pt);
        return temp;
    }

    friend ostream &operator<<(ostream &output, Point3 pt) {
        output<<pt.x<<","<<pt.y<<","<<pt.z<<endl;
        return output;
    }

    Point3 normalize() {
        return *this / sqrt(x*x + y*y + z*z);
    }

};

class Ray {

public:

    Point3 start;
    Point3 dir;

    Ray(Point3 start, Point3 dir) {
        this->start = start;
        this->dir = dir.normalize();
    }
};

enum {AMBIENT, DIFFUSE, SPECULAR, REFLECTION};

class Object {

public:

    Point3 reference_point;

    double height, width, length, source_factor = 1.0, refIdx = 1.5;

    int shine;

    double color[3];

    double co_efficients[4];

    Object() {}

    virtual void draw() = 0;

    virtual double getIntersectionT(Ray *r, bool debug) = 0;

    virtual Point3 getNormal(Point3 intersection) = 0;

    virtual double intersect(Ray* r, double current_color[3], int level) = 0;

    void setColor(double r, double g, double b) {
        color[0] = r;
        color[1] = g;
        color[2] = b;
    }

    void setShine(int shine) {
        this->shine = shine;
    }

    void setCoEfficients(double a, double d, double s, double r) {
        co_efficients[AMBIENT] = a;
        co_efficients[DIFFUSE] = d;
        co_efficients[SPECULAR] = s;
        co_efficients[REFLECTION] = r;
    }

    double dotProduct(Point3 a, Point3 b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }

    Point3 crossProduct(Point3 u, Point3 v) {
        Point3 prod(u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);
        return prod;
    }

    Point3 getReflection(Ray* ray, Point3 normal) {
        Point3 reflection = ray->dir - normal * 2.0 * dotProduct(ray->dir, normal);
        return reflection.normalize();
    }

    Point3 getRefraction(Ray* ray, Point3 normal) {

        Point3 refraction(0, 0, 0);

        double dot = dotProduct(normal, ray->dir);
        double k = 1.0 - refIdx * refIdx * (1.0 - dot * dot);

        if (k >= 0) {
            refraction = ray->dir * refIdx - normal * (refIdx * dot + sqrt(k));
            refraction = refraction.normalize();
        }

        return refraction;
    }

};

vector<Object*> objects;
vector<Point3> lights;

class Sphere: public Object {

public:

    Sphere(Point3 center, double radius) {
        reference_point = center;
        length = radius;
    }

    void draw() {

        glColor3f(color[0], color[1], color[2]);

        Point3 points[100][100];

        double h, r;

        int slices = 24, stacks = 20;

        //generate points
        for(int i=0; i<=stacks; i++) {
            h = length * sin(((double)i/(double)stacks)*(pi/2));
            r = length * cos(((double)i/(double)stacks)*(pi/2));
            for(int j=0; j<=slices; j++) {
                points[i][j].x=r*cos(((double)j/(double)slices)*2*pi);
                points[i][j].y=r*sin(((double)j/(double)slices)*2*pi);
                points[i][j].z=h;
            }
        }

        //draw quads using generated points
        for(int i=0; i<stacks; i++) {
            for(int j=0; j<slices; j++) {
                glBegin(GL_QUADS);
                {
                    //upper hemisphere
                    glVertex3f(points[i][j].x+reference_point.x,points[i][j].y+reference_point.y,points[i][j].z+reference_point.z);
                    glVertex3f(points[i][j+1].x+reference_point.x,points[i][j+1].y+reference_point.y,points[i][j+1].z+reference_point.z);
                    glVertex3f(points[i+1][j+1].x+reference_point.x,points[i+1][j+1].y+reference_point.y,points[i+1][j+1].z+reference_point.z);
                    glVertex3f(points[i+1][j].x+reference_point.x,points[i+1][j].y+reference_point.y,points[i+1][j].z+reference_point.z);
                    //lower hemisphere
                    glVertex3f(points[i][j].x+reference_point.x,points[i][j].y+reference_point.y,-points[i][j].z+reference_point.z);
                    glVertex3f(points[i][j+1].x+reference_point.x,points[i][j+1].y+reference_point.y,-points[i][j+1].z+reference_point.z);
                    glVertex3f(points[i+1][j+1].x+reference_point.x,points[i+1][j+1].y+reference_point.y,-points[i+1][j+1].z+reference_point.z);
                    glVertex3f(points[i+1][j].x+reference_point.x,points[i+1][j].y+reference_point.y,-points[i+1][j].z+reference_point.z);
                }
                glEnd();
            }
        }
    }

    double getIntersectionT(Ray* ray, bool debug) {

        Point3 start = ray->start - reference_point;
        //start = start.normalize();

        double a = dotProduct(ray->dir, ray->dir);
        double b = 2 * dotProduct(ray->dir, start);
        double c = dotProduct(start, start) - length*length;

        double d = b*b - 4*a*c;


        if (d < 0) {
            return -1;
        }

        double t1 = (- b + sqrt(d)) / (2.0*a);
        double t2 = (- b - sqrt(d)) / (2.0*a);

        if (t2 > t1) {
            double temp = t1;
            t1 = t2;
            t2 = temp;
        }

        return t2;
    }

    Point3 getNormal(Point3 intersection) {
        Point3 normal = intersection - reference_point;
        return normal.normalize();
    }

    double intersect(Ray* ray, double current_color[3], int level) {

        double t = getIntersectionT(ray, false);

        if (t <= 0) {
            return -1;
        }

        if (level == 0 ) {
            return t;
        }


        for (int i=0; i<3; i++) {
            current_color[i] = color[i] * co_efficients[AMBIENT];
        }


        Point3 intersectionPoint = ray->start + ray->dir * t;

        Point3 normal = getNormal(intersectionPoint);

        Point3 reflection = getReflection(ray, normal);

        Point3 refraction = getRefraction(ray, normal);

        for (int i=0; i<lights.size(); i++) {

            Point3 dir = lights[i] - intersectionPoint;
            double len = sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
            dir = dir.normalize();

            Point3 start = intersectionPoint + dir*1.0;
            Ray L(start, dir);
            //cout<<intersectionPoint<<L.start<<L.dir;

            bool flag = false;

            for (int j=0; j < objects.size(); j++) {

                double tObj = objects[j]->getIntersectionT(&L, true);

                if(tObj > 0 || abs(tObj) > len) {
                    continue;
                }

                flag = true;
                break;
            }

            if (flag){

                double lambert = dotProduct(L.dir, normal);
                double phong = pow(dotProduct(reflection, ray->dir), shine);

                lambert = lambert > 0 ? lambert : 0;
                phong = phong > 0 ? phong : 0;

                for (int k=0; k<3; k++) {
                    current_color[k] += source_factor * lambert * co_efficients[DIFFUSE] * color[k];
                    current_color[k] += source_factor * phong * co_efficients[SPECULAR] * color[k];
                }
            }

            if (level < recursion_level) {

                start = intersectionPoint + reflection * 1.0;

                Ray reflectionRay(start, reflection);

                int nearest=-1;
                double minT = 9999999;
                double reflected_color[3];

                for (int k=0; k < objects.size(); k++) {

                    double tk = objects[k]->getIntersectionT(&reflectionRay, true);

                    if(tk <= 0) {
                        continue;
                    } else if (tk < minT) {
                        minT = tk;
                        nearest = k;
                    }

                    //cout<<tk<<endl;
                }

                if(nearest!=-1) {

                    objects[nearest]->intersect(&reflectionRay, reflected_color, level+1);

                    for (int k=0; k<3; k++) {
                        current_color[k] += reflected_color[k] * co_efficients[REFLECTION];
                    }
                }

                start = intersectionPoint + refraction * 1.0;

                Ray refractionRay(start, refraction);

                nearest=-1;
                minT = 9999999;
                double refracted_color[3];

                for (int k=0; k < objects.size(); k++) {

                    double tk = objects[k]->getIntersectionT(&refractionRay, true);

                    if(tk <= 0) {
                        continue;
                    } else if (tk < minT) {
                        minT = tk;
                        nearest = k;
                    }

                    //cout<<tk<<endl;
                }

                if(nearest!=-1) {

                    objects[nearest]->intersect(&refractionRay, refracted_color, level+1);

                    for (int k=0; k<3; k++) {
                        current_color[k] += refracted_color[k] * refIdx;
                    }
                }
            }

            for (int k=0; k<3; k++) {
                if (current_color[k] > 1) {
                    current_color[k] = 1;
                } else if (current_color[k] < 0) {
                    current_color[k] = 0;
                }
            }


        }

        return t;
    }


};

class Floor: public Object {

public:

    bitmap_image bd;
    double bdHeight, bdWidth;

    Floor(double floorWidth, double tileWidth) {
        reference_point = Point3(-floorWidth/2, -floorWidth/2, 0);
        length = tileWidth;
        bd = bitmap_image("bd.bmp");
        bdHeight = bd.height()/1000.0;
        bdWidth = bd.width()/1000.0;
    }

    void draw() {

        int numOfTiles = abs(reference_point.x*2/length);
        int seq = 0;

        for (int i=0; i<numOfTiles; i++) {
            for (int j=0; j<numOfTiles; j++) {

                if ((i+j)%2) {
                    glColor3f(0, 0, 0);
                } else {
                    glColor3f(1, 1, 1);
                }

                glBegin(GL_QUADS);
                {
                    glVertex3f(reference_point.x+length*i, reference_point.y+length*j, reference_point.z);
                    glVertex3f(reference_point.x+length*(i+1), reference_point.y+length*j, reference_point.z);
                    glVertex3f(reference_point.x+length*(i+1), reference_point.y+length*(j+1), reference_point.z);
                    glVertex3f(reference_point.x+length*i, reference_point.y+length*(j+1), reference_point.z);
                }
                glEnd();
            }
        }
    }

    Point3 getNormal(Point3 intersection) {
        Point3 normal(0,0,1);
        return normal.normalize();
    }

    double getIntersectionT(Ray* ray, bool debug) {

        Point3 normal = getNormal(reference_point);

        double t = dotProduct(normal, ray->start) * (-1) / dotProduct(normal, ray->dir);

        return t;
    }


    double intersect(Ray* ray, double current_color[3], int level) {

        double t = getIntersectionT(ray, false);

        Point3 intersectionPoint = ray->start + ray->dir * t;

        double xMin = reference_point.x;
        double xMax = xMin * (-1);

        double yMin = reference_point.y;
        double yMax = yMin * (-1);



        if (xMin > intersectionPoint.x || intersectionPoint.x > xMax ||
                yMin > intersectionPoint.y || intersectionPoint.y > yMax) {
            return -1;
        }

        //cout<<xMin<<','<<xMax<<';'<<yMin<<','<<yMax<<endl;
        //cout<<intersectionPoint;

        int xCord = intersectionPoint.x / length;
        int yCord = intersectionPoint.y / length;

        if ((xCord+yCord)%2) {
            color[0] = color[1] = color[2] = 0;
        } else {
            color[0] = color[1] = color[2] = 1;
        }

        unsigned char r, g, b;
        int x = (intersectionPoint.x + abs(reference_point.x)) * bdWidth;
        int y = (intersectionPoint.y + abs(reference_point.y)) * bdHeight;

        //cout<<x<<','<<y<<endl;

        bd.get_pixel(x, y, r, g, b);

        double rgb[] = {r, g, b};
        //cout<<rgb[0];

        for (int i=0; i<3; i++) {
            current_color[i] = color[i] * co_efficients[AMBIENT] * rgb[i] / 255.0;
        }

        Point3 normal = getNormal(intersectionPoint);

        Point3 reflection = getReflection(ray, normal);
        Point3 refraction = getRefraction(ray, normal);

        for (int i=0; i<lights.size(); i++) {

            Point3 dir = lights[i] - intersectionPoint;
            double len = sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
            dir = dir.normalize();

            Point3 start = intersectionPoint + dir*1.0;
            Ray L(start, dir);
            //cout<<intersectionPoint<<L.start<<L.dir;

            bool flag = false;

            for (int j=0; j < objects.size(); j++) {

                double tObj = objects[j]->getIntersectionT(&L, true);

                if(tObj > 0 || abs(tObj) > len) {
                    continue;
                }

                flag = true;
                break;
            }

            if (flag){

                double lambert = dotProduct(L.dir, normal);
                double phong = pow(dotProduct(reflection, ray->dir), shine);

                lambert = lambert > 0 ? lambert : 0;
                phong = phong > 0 ? phong : 0;

                for (int k=0; k<3; k++) {
                    current_color[k] += source_factor * lambert * co_efficients[DIFFUSE] * color[k];
                    current_color[k] += source_factor * phong * co_efficients[SPECULAR] * color[k];
                }
            }

            if (level < recursion_level) {

                start = intersectionPoint + reflection * 1.0;

                Ray reflectionRay(start, reflection);

                int nearest=-1;
                double minT = 9999999;
                double reflected_color[3];

                for (int k=0; k < objects.size(); k++) {

                    double tk = objects[k]->getIntersectionT(&reflectionRay, true);

                    if(tk <= 0) {
                        continue;
                    } else if (tk < minT) {
                        minT = tk;
                        nearest = k;
                    }

                    //cout<<tk<<endl;
                }

                if(nearest!=-1) {

                    objects[nearest]->intersect(&reflectionRay, reflected_color, level+1);

                    for (int k=0; k<3; k++) {
                        current_color[k] += reflected_color[k] * co_efficients[REFLECTION];
                    }
                }

                start = intersectionPoint + refraction * 1.0;

                Ray refractionRay(start, refraction);

                nearest=-1;
                minT = 9999999;
                double refracted_color[3];

                for (int k=0; k < objects.size(); k++) {

                    double tk = objects[k]->getIntersectionT(&refractionRay, true);

                    if(tk <= 0) {
                        continue;
                    } else if (tk < minT) {
                        minT = tk;
                        nearest = k;
                    }

                    //cout<<tk<<endl;
                }

                if(nearest!=-1) {

                    objects[nearest]->intersect(&refractionRay, refracted_color, level+1);

                    for (int k=0; k<3; k++) {
                        current_color[k] += refracted_color[k] * refIdx;
                    }
                }
            }

            for (int k=0; k<3; k++) {
                if (current_color[k] > 1) {
                    current_color[k] = 1;
                } else if (current_color[k] < 0) {
                    current_color[k] = 0;
                }
            }


        }

        return t;
    }


};

class Triangle: public Object {

public:

    Point3 a, b, c;

    Triangle(Point3 a, Point3 b, Point3 c) {
        this->a = a;
        this->b = b;
        this->c = c;
    }

    void draw() {
        glColor3f(color[0],color[1],color[2]);
        glBegin(GL_TRIANGLES);
        {
            glVertex3f(a.x, a.y, a.z);
            glVertex3f(b.x, b.y, b.z);
            glVertex3f(c.x, c.y, c.z);
        }
        glEnd();
    }

    Point3 getNormal(Point3 intersection) {

        Point3 u = b - a;
        Point3 v = c - a;

        Point3 normal = crossProduct(u, v);

        return normal.normalize();
    }

    double getIntersectionT(Ray* ray, bool debug) {
        Point3 e1 = b - a;
        Point3 e2 = c - a;

        Point3 p = crossProduct(ray->dir, e2);
        double det = dotProduct(e1, p);

        if(det > -EPSILON && det < EPSILON) {
            return -1;
        }

        double inv_det = 1.0 / det;

        Point3 T = ray->start - a;

        double u = dotProduct(T, p) * inv_det;

        if(u < 0 || u > 1) {
            return -1;
        }

        Point3 q = crossProduct(T, e1);

        double v = dotProduct(ray->dir, q) * inv_det;

        if(v < 0 || u + v  > 1) {
            return -1;
        }

        double t = dotProduct(e2, q) * inv_det;

        if(t > EPSILON) { //ray intersection
            return t;
        }

        return -1;
    }


    double intersect(Ray* ray, double current_color[3], int level) {

        double t = getIntersectionT(ray, false);

        if (t <= 0) {
            return -1;
        }

        if (level == 0 ) {
            return t;
        }


        for (int i=0; i<3; i++) {
            current_color[i] = color[i] * co_efficients[AMBIENT];
        }


        Point3 intersectionPoint = ray->start + ray->dir * t;

        for (int i=0; i<lights.size(); i++) {

            Point3 normal = getNormal(intersectionPoint);

            Point3 dir = lights[i] - intersectionPoint;
            double len = sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
            dir = dir.normalize();

            if (dotProduct(dir, normal) > 0) {
                normal = normal * (-1);
            }

            Point3 reflection = getReflection(ray, normal);
            Point3 refraction = getRefraction(ray, normal);

            Point3 start = intersectionPoint + dir*1.0;
            Ray L(start, dir);
            //cout<<intersectionPoint<<L.start<<L.dir;

            bool flag = false;

            for (int j=0; j < objects.size(); j++) {

                double tObj = objects[j]->getIntersectionT(&L, true);

                if(tObj > 0 || abs(tObj) > len) {
                    continue;
                }

                flag = true;
                break;
            }

            if (flag){

                double lambert = dotProduct(L.dir, normal);
                double phong = pow(dotProduct(reflection, ray->dir), shine);

                lambert = lambert > 0 ? lambert : 0;
                phong = phong > 0 ? phong : 0;

                for (int k=0; k<3; k++) {
                    current_color[k] += source_factor * lambert * co_efficients[DIFFUSE] * color[k];
                    current_color[k] += source_factor * phong * co_efficients[SPECULAR] * color[k];
                }
            }

            if (level < recursion_level) {

                start = intersectionPoint + reflection * 1.0;

                Ray reflectionRay(start, reflection);

                int nearest=-1;
                double minT = 9999999;
                double reflected_color[3];

                for (int k=0; k < objects.size(); k++) {

                    double tk = objects[k]->getIntersectionT(&reflectionRay, true);

                    if(tk <= 0) {
                        continue;
                    } else if (tk < minT) {
                        minT = tk;
                        nearest = k;
                    }

                    //cout<<tk<<endl;
                }

                if(nearest!=-1) {

                    objects[nearest]->intersect(&reflectionRay, reflected_color, level+1);

                    for (int k=0; k<3; k++) {
                        current_color[k] += reflected_color[k] * co_efficients[REFLECTION];
                    }
                }

                start = intersectionPoint + refraction * 1.0;

                Ray refractionRay(start, refraction);

                nearest=-1;
                minT = 9999999;
                double refracted_color[3];

                for (int k=0; k < objects.size(); k++) {

                    double tk = objects[k]->getIntersectionT(&refractionRay, true);

                    if(tk <= 0) {
                        continue;
                    } else if (tk < minT) {
                        minT = tk;
                        nearest = k;
                    }

                    //cout<<tk<<endl;
                }

                if(nearest!=-1) {

                    objects[nearest]->intersect(&refractionRay, refracted_color, level+1);

                    for (int k=0; k<3; k++) {
                        current_color[k] += refracted_color[k] * refIdx;
                    }
                }
            }

            for (int k=0; k<3; k++) {
                if (current_color[k] > 1) {
                    current_color[k] = 1;
                } else if (current_color[k] < 0) {
                    current_color[k] = 0;
                }
            }


        }

        return t;
    }



};


class GeneralQuadratic: public Object {

public:

    double A, B, C, D, E, F, G, H, I, J;

    GeneralQuadratic(double coeff[10], Point3 reff, double length, double width, double height) {
        this->A = coeff[0];
        this->B = coeff[1];
        this->C = coeff[2];
        this->D = coeff[3];
        this->E = coeff[4];
        this->F = coeff[5];
        this->G = coeff[6];
        this->H = coeff[7];
        this->I = coeff[8];
        this->J = coeff[9];
        this->reference_point = reff;
        this->height = height;
        this->width = width;
        this->length = length;
    }

    void draw() {}

    Point3 getNormal(Point3 intersection) {

        double u = 2 * A * intersection.x + D * intersection.y + F * intersection.z  + G;
        double v = 2 * B * intersection.y + D * intersection.x + E * intersection.z  + H;
        double z = 2 * C * intersection.z + E * intersection.y + F * intersection.x  + I;

        Point3 normal(u, v, z);

        return normal.normalize();
    }

    double getIntersectionT(Ray* ray, bool debug) {

        double a = A * ray->dir.x * ray->dir.x + B * ray->dir.y * ray->dir.y + C * ray->dir.z * ray->dir.z;
        double b = 2 * (A * ray->start.x * ray->dir.x + B * ray->start.y * ray->dir.y + C * ray->start.z * ray->dir.z);
        double c = A * ray->start.x * ray->start.x + B * ray->start.y * ray->start.y + C * ray->start.z * ray->start.z;

        a += D * ray->dir.x * ray->dir.y + E * ray->dir.y * ray->dir.z + F * ray->dir.z * ray->dir.x;
        b += D * (ray->start.x * ray->dir.y + ray->dir.x * ray->start.y)
             + E * (ray->start.y * ray->dir.z + ray->dir.y * ray->start.z)
             + F * (ray->start.z * ray->dir.x + ray->dir.z * ray->start.x);
        c += D * ray->start.x * ray->start.y + E * ray->start.y * ray->start.z + F * ray->start.z * ray->start.x;

        b += G * ray->dir.x + H * ray->dir.y + I * ray->dir.z;
        c += G * ray->start.x + H * ray->start.y + I * ray->start.z + J;


        double d = b*b - 4*a*c;


        if (d < 0) {
            return -1;
        }

        double t1 = (- b + sqrt(d)) / (2.0*a);
        double t2 = (- b - sqrt(d)) / (2.0*a);


        Point3 intersectionPoint1 = ray->start + ray->dir * t1;
        Point3 intersectionPoint2 = ray->start + ray->dir * t2;

        double xMin = reference_point.x;
        double xMax = xMin + length;

        double yMin = reference_point.y;
        double yMax = yMin + width;

        double zMin = reference_point.z;
        double zMax = zMin + height;

        //cout<<xMin<<','<<xMax<<';'<<yMin<<','<<yMax<<';'<<zMin<<','<<zMax<<endl;
        //cout<<intersectionPoint1<<intersectionPoint2;

        bool flag1 = (length > 0 && (xMin > intersectionPoint1.x || intersectionPoint1.x > xMax) ||
                      width > 0 && (yMin > intersectionPoint1.y || intersectionPoint1.y > yMax) ||
                      height > 0 && (zMin > intersectionPoint1.z || intersectionPoint1.z > zMax));

        bool flag2 = (length > 0 && (xMin > intersectionPoint2.x || intersectionPoint2.x > xMax) ||
                      width > 0 && (yMin > intersectionPoint2.y || intersectionPoint2.y > yMax) ||
                      height > 0 && (zMin > intersectionPoint2.z || intersectionPoint2.z > zMax));

        //cout<<flag1<<','<<flag2<<endl;

        if (flag1 && flag2) {
            return -1;
        } else if (flag1) {
            return t2;
        } else if (flag2) {
            return t1;
        } else {
            if (t2 > t1) {
                double temp = t1;
                t1 = t2;
                t2 = temp;
            }
            return t2;
        }
    }


    double intersect(Ray* ray, double current_color[3], int level) {

        double t = getIntersectionT(ray, false);

        if (t <= 0) {
            return -1;
        }

        if (level == 0 ) {
            return t;
        }


        for (int i=0; i<3; i++) {
            current_color[i] = color[i] * co_efficients[AMBIENT];
        }


        //cout<<t<<endl;
        Point3 intersectionPoint = ray->start + ray->dir * t;

        Point3 normal = getNormal(intersectionPoint);

        Point3 reflection = getReflection(ray, normal);

        Point3 refraction = getRefraction(ray, normal);

        for (int i=0; i<lights.size(); i++) {

            Point3 dir = lights[i] - intersectionPoint;
            double len = sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
            dir = dir.normalize();

            Point3 start = intersectionPoint + dir*1.0;
            Ray L(start, dir);
            //cout<<intersectionPoint<<L.start<<L.dir;

            bool flag = false;

            for (int j=0; j < objects.size(); j++) {

                double tObj = objects[j]->getIntersectionT(&L, true);

                if(tObj > 0 || abs(tObj) > len) {
                    continue;
                }

                flag = true;
                break;
            }

            if (flag){

                double lambert = dotProduct(L.dir, normal);
                double phong = pow(dotProduct(reflection, ray->dir), shine);

                lambert = lambert > 0 ? lambert : 0;
                phong = phong > 0 ? phong : 0;

                for (int k=0; k<3; k++) {
                    current_color[k] += source_factor * lambert * co_efficients[DIFFUSE] * color[k];
                    current_color[k] += source_factor * phong * co_efficients[SPECULAR] * color[k];
                }
            }

            if (level < recursion_level) {

                start = intersectionPoint + reflection * 1.0;

                Ray reflectionRay(start, reflection);

                int nearest=-1;
                double minT = 9999999;
                double reflected_color[3];

                for (int k=0; k < objects.size(); k++) {

                    double tk = objects[k]->getIntersectionT(&reflectionRay, true);

                    if(tk <= 0) {
                        continue;
                    } else if (tk < minT) {
                        minT = tk;
                        nearest = k;
                    }

                    //cout<<tk<<endl;
                }

                if(nearest!=-1) {

                    objects[nearest]->intersect(&reflectionRay, reflected_color, level+1);

                    for (int k=0; k<3; k++) {
                        current_color[k] += reflected_color[k] * co_efficients[REFLECTION];
                    }
                }

                start = intersectionPoint + refraction * 1.0;

                Ray refractionRay(start, refraction);

                nearest=-1;
                minT = 9999999;
                double refracted_color[3];

                for (int k=0; k < objects.size(); k++) {

                    double tk = objects[k]->getIntersectionT(&refractionRay, true);

                    if(tk <= 0) {
                        continue;
                    } else if (tk < minT) {
                        minT = tk;
                        nearest = k;
                    }

                    //cout<<tk<<endl;
                }

                if(nearest!=-1) {

                    objects[nearest]->intersect(&refractionRay, refracted_color, level+1);

                    for (int k=0; k<3; k++) {
                        current_color[k] += refracted_color[k] * refIdx;
                    }
                }
            }

            for (int k=0; k<3; k++) {
                if (current_color[k] > 1) {
                    current_color[k] = 1;
                } else if (current_color[k] < 0) {
                    current_color[k] = 0;
                }
            }


        }

        return t;
    }



};

#endif // BASE_HPP_INCLUDED
