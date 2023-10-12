Point(1) = {0, 0, 0, edgeLength};
Point(2) = {9, 0, 0, edgeLength};
Point(3) = {9, 0, 6, edgeLength};
Point(4) = {0, 0, 6, edgeLength};

Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 1};

Line Loop(11) = {1,2,3,4};

Plane Surface(11) = {11};

