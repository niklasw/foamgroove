function versionIsNewerThan()                                                       
{                                                                               
    [[ -z "$WM_PROJECT_VERSION" ]] && { echo "FOAM not correctlu loaded"; exit 1; }       
    test $(echo -e "$1\n$WM_PROJECT_VERSION" | sort -V|head -1) != $1                    
    echo $?                                                                     
}                                                                               
  
runme()
{
    echo -e "Running: $@"
    [[ -d logs ]] || mkdir logs
    "$@" > logs/$1.log
    if [ $? != "0" ]; then
        cat logs/$1.log
        echo -e "Some error! Exiting."
        exit 1
    fi
}

newTempCase()
{
    [[ -d $1 ]] && runme rm -rf $1 ;
    runme mkdir $1;
    ( touch $1/case.foam )
    runme cp -r setup.foam constant system $1/.
}


extrude()
{
    ( set -x; cd system; ln -sf $1 extrudeMeshDict )
    runme extrudeMesh
}

merge()
{
    runme mergeMeshes $1 $2 -overwrite
}

stitch()
{
    runme stitchMesh $1 $2 -case $3 -overwrite
}

mergeAndStitch()
{
    merge $1 $2
    stitch $3 $4 $1
}

newBlock()
{
    cleanMesh
    runme blockMesh -dict system/$1
}

getMesh()
{
    cleanMesh
    runme mv $1/constant/polyMesh/* constant/polyMesh/.
}

cleanMesh()
{
    if [ $(versionIsNewerThan 2.9.9) == 1 ]; then
        runme foamCleanPolyMesh
    else
        runme foamClearPolyMesh
    fi
}

