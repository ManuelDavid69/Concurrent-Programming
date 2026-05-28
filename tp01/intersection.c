#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    double x;
    double y;
} Point;

typedef struct {
    Point first;
    Point second;
    bool vertical;
    double slope;
    double intercept;
} Line;

static const double EPSILON = 1e-9;

static bool nearly_equal(double left, double right)
{
    return fabs(left - right) <= EPSILON * (1.0 + fabs(left) + fabs(right));
}

/*
 * Renvoie le coefficient directeur de la droite passant par deux points.  Les
 * droites verticales n'ont pas de coefficient directeur : la fonction renvoie
 * false et l'appelant utilise first.x à la place.
 */
static bool coefficient(Point first, Point second, double *slope)
{
    if (nearly_equal(first.x, second.x)) {
        return false;
    }
    *slope = (second.y - first.y) / (second.x - first.x);
    return true;
}

/* Ordonnée à l'origine de la droite passant par un point avec le coefficient donné. */
static double ordonnee(double slope, Point point)
{
    return point.y - slope * point.x;
}

static Line build_line(Point first, Point second)
{
    Line line;

    line.first = first;
    line.second = second;
    line.vertical = !coefficient(first, second, &line.slope);
    line.intercept = line.vertical ? 0.0 : ordonnee(line.slope, first);
    return line;
}

static bool between(double value, double left, double right)
{
    double minimum = left < right ? left : right;
    double maximum = left < right ? right : left;
    return value >= minimum - EPSILON && value <= maximum + EPSILON;
}

static bool on_segment(const Line *line, Point point)
{
    return between(point.x, line->first.x, line->second.x)
        && between(point.y, line->first.y, line->second.y);
}

static int intersection(const Line *first, const Line *second)
{
    Point point;

    if (first->vertical && second->vertical) {
        if (!nearly_equal(first->first.x, second->first.x)) {
            printf("Aucune intersection (droites parallèles)\n");
            return EXIT_SUCCESS;
        }
        printf("Infinité d'intersections (droites confondues)\n");
        return EXIT_SUCCESS;
    }

    if (first->vertical) {
        point.x = first->first.x;
        point.y = second->slope * point.x + second->intercept;
    } else if (second->vertical) {
        point.x = second->first.x;
        point.y = first->slope * point.x + first->intercept;
    } else {
        if (nearly_equal(first->slope, second->slope)) {
            if (nearly_equal(first->intercept, second->intercept)) {
                printf("Infinité d'intersections (droites confondues)\n");
            } else {
                printf("Aucune intersection (droites parallèles)\n");
            }
            return EXIT_SUCCESS;
        }
        point.x = (second->intercept - first->intercept) / (first->slope - second->slope);
        point.y = first->slope * point.x + first->intercept;
    }

    printf("Intersection des droites: (%.6f, %.6f)\n", point.x, point.y);
    if (on_segment(first, point) && on_segment(second, point)) {
        printf("Le point appartient aux deux segments\n");
    } else {
        printf("Le point est hors de l'un des deux segments\n");
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    Point p1;
    Point p2;
    Point p3;
    Point p4;
    Line first;
    Line second;
    double coordinates[8];
    int index;

    if (argc != 9) {
        fprintf(stderr,
                "usage: %s x1 y1 x2 y2 x3 y3 x4 y4\n"
                "       (P1, P2 on the first line; P3, P4 on the second)\n",
                argv[0]);
        return EXIT_FAILURE;
    }
    for (index = 0; index < 8; index++) {
        if (!parse_double(argv[index + 1], &coordinates[index])) {
            fprintf(stderr, "%s: not a number\n", argv[index + 1]);
            return EXIT_FAILURE;
        }
    }
    p1 = (Point){coordinates[0], coordinates[1]};
    p2 = (Point){coordinates[2], coordinates[3]};
    p3 = (Point){coordinates[4], coordinates[5]};
    p4 = (Point){coordinates[6], coordinates[7]};

    if (nearly_equal(p1.x, p2.x) && nearly_equal(p1.y, p2.y)) {
        fprintf(stderr, "P1 and P2 are identical; the first line is undefined\n");
        return EXIT_FAILURE;
    }
    if (nearly_equal(p3.x, p4.x) && nearly_equal(p3.y, p4.y)) {
        fprintf(stderr, "P3 and P4 are identical; the second line is undefined\n");
        return EXIT_FAILURE;
    }

    first = build_line(p1, p2);
    second = build_line(p3, p4);
    return intersection(&first, &second);
}
