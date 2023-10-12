nCells = $__casesize;
edgeLength = 1.0/nCells;
Point(1) = {0, 0, 0, edgeLength};
Point(2) = {1, 0, 0, edgeLength};
Point(3) = {1, 1, 0, edgeLength};
Point(4) = {0, 1, 0, edgeLength};
Point(5) = {1, 1, 1, edgeLength};
Point(6) = {0, 1, 1, edgeLength};
Point(10) = {0, 0, 1, edgeLength};
Point(14) = {1, 0, 1, edgeLength};
//
Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 1};
Line(13) = {5, 6};
Line(14) = {6, 10};
Line(15) = {10, 14};
Line(16) = {14, 5};
Line(18) = {3, 5};
Line(19) = {4, 6};
Line(23) = {1, 10};
Line(27) = {2, 14};
//
Line Loop(11) = {3, 4, 1, 2};
Plane Surface(11) = {11};
Line Loop(20) = {3, 19, -13, -18};
Ruled Surface(20) = {20};
Line Loop(24) = {4, 23, -14, -19};
Ruled Surface(24) = {24};
Line Loop(28) = {1, 27, -15, -23};
Ruled Surface(28) = {28};
Line Loop(32) = {2, 18, -16, -27};
Ruled Surface(32) = {32};
Line Loop(33) = {13, 14, 15, 16};
Plane Surface(33) = {33};
//
Surface Loop(1) = {11, 20, 24, 28, 32, 33};
Volume(1) = {1};
//
Physical Surface("ymin") = {28};
Physical Surface("ymax") = {20};
Physical Surface("zmin") = {11};
Physical Surface("xmax") = {32};
Physical Surface("zmax") = {33};
Physical Surface("xmin") = {24};
Physical Volume("internal") = {1};

Periodic Surface 28 {1, 27, -15, -23} = 20 {-3, 18, 13, -19};
Periodic Surface 24 {4, 23, -14, -19} = 32 {-2, 27, 16, -18};
Periodic Surface 11 {3, 4, 1, 2} = 33 {13, 14, 15, 16};

