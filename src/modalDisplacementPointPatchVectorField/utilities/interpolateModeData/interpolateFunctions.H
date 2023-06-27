#ifndef INTERPOLATEMODEFUNCTIONS
#define INTERPOLATEMODEFUNCTIONS
const pointField& getPatchPoints(const fvMesh& mesh, const word patchName)
{
    label patchLabel = mesh.boundaryMesh().findPatchID(patchName);
    if (patchLabel < 0)
    {
        FatalErrorIn("getPatchPoints")
            << "Patch name not found in case: "
            << patchName
            << exit(FatalError);
    }

    const polyPatch& patch = mesh.boundaryMesh()[patchLabel];
    const pointField& points =  patch.localPoints();

    return points;
}

// If argument point list is the complete point list
// I.e. not a subset.
label closest(const point& p, const List<point>& points)
{
    scalar dist=1e9;
    label index=-1;
    forAll (points, i)
    {
        scalar d = mag(points[i]-p);
        if (d < dist)
        {
            dist = d;
            index = i;
        }
    }

    return index;
}

label closest(const point& p, const List<point>& points, const List<label>& labels)
{
    scalar dist=1e9;
    label index=-1;
    forAll (labels, i)
    {
        scalar d = mag(points[labels[i]]-p);
        if (d < dist)
        {
            dist = d;
            index = labels[i];
        }
    }

    return index;
}

List<label> rawClosestSearch
(
    const List<point>& patchPoints,
    const List<point>& coordinates,
    bool debug=false
)
{
    List<label> modeLabels(patchPoints.size());

    forAll (patchPoints, pointI)
    {
        const point& p = patchPoints[pointI];
        modeLabels[pointI] = closest(p,coordinates);
    }

    if (debug)
    {
        List<scalar> distances(patchPoints.size());
        forAll (patchPoints,pointI)
        {
            distances[pointI] = mag(patchPoints[pointI] - coordinates[modeLabels[pointI]]);
        }

        Info << "Min distance = " << min(distances) << endl;
        Info << "Max distance = " << max(distances) << endl;
        Info << "Average distance = " << average(distances) << endl;
    }

    return modeLabels;
}

List<label> pointSearch
(
    const List<point>& patchPoints,
    const List<point>& coordinates,
    bool debug=false
)
{
    return rawClosestSearch(patchPoints, coordinates, debug);
}


vectorField patchPointDisplacements
(
    const List<point>& patchPoints,
    const List<label>& modeLabels,
    const List<vector>& displacements
)
{
    vectorField patchDisplacements(patchPoints.size());

    forAll (modeLabels,i)
    {
        patchDisplacements[i] = displacements[modeLabels[i]];
    }

    return patchDisplacements;
}

void transferMode
(
    const word modeName,
    const fvMesh& mesh,
    const word& patchName,
    modeFile& Mode,
    List<label>& modeLabels
)
{
    Mode.read();

    const List<vector>& displacements = Mode.displacements();

    const pointField& patchPoints = getPatchPoints(mesh,patchName);

    //- Use old modeLabels if they exists.
    if ( modeLabels.size() == 0)
    {
        modeLabels.append(pointSearch(patchPoints, Mode.coordinates()));
    }

    vectorField patchDisplacements = patchPointDisplacements
                                     (
                                        patchPoints,
                                        modeLabels,
                                        displacements
                                     );

    Info << "\nWriting patch point displacements for patch "
         << patchName << "\n" << endl;

    word fName = "interpolated_"+modeName+"_"+patchName;
    fileName fPath = fileName(mesh.time().constant()/"structuralModes");

    OFstream dispOF(fPath/fName);
    dispOF << patchDisplacements;
}

void writeMode
(
    const vectorField& mode,
    const word& fName,
    const fileName& fPath
)
{
    // To be implemented.    
}

scalar RBF(const scalar r, const scalar eps)
{
    return 1/(1+pow(eps*r,2));
}

void transferModeRBF
(
    const word modeName,
    const fvMesh& mesh,
    const word& patchName,
    modeFile& Mode
)
{
    FatalErrorIn("transferModeRBF") << "NOT IMPLEMENTED" << exit(FatalError);

    Mode.read();
    const vectorField& displacements = Mode.displacements();
    const pointField& coords = Mode.coordinates();

    const pointField& patchPoints = getPatchPoints(mesh,patchName);

    vectorField patchDisplacements(patchPoints.size(),vector::zero);

    forAll(patchPoints, ppI)
    {
        const point& pp = patchPoints[ppI];
        forAll(coords, i)
        {
            scalar r = Foam::mag(coords[i]-pp);
            patchDisplacements[ppI] += displacements[i] * RBF(r,1e-3);
        }
    }

    Info << "\nWriting patch point displacements for patch "
         << patchName << "\n" << endl;

    word fName = "rbfInterpolated_"+modeName+"_"+patchName;
    fileName fPath = fileName(mesh.time().constant()/"structuralModes");

    OFstream dispOF(fPath/fName);
    dispOF << patchDisplacements;
}
#endif
