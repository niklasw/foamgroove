function versionIsNewerThan()                                                       
{                                                                               
    [[ -z "$WM_PROJECT_VERSION" ]] && { echo "FOAM not correctlu loaded"; exit 1; }       
    test $(echo -e "$1\n$WM_PROJECT_VERSION" | sort -V|head -1) != $1                    
    echo $?                                                                     
}                                                                               

#- Just a wrapper that informs and pipes stdout to log.
runme()
{
    echo -e "Running: $@"
    [[ -d logs ]] || mkdir logs
    [[ "$1" == "mpirun" ]] &&  log=$4 || log=$1
    "$@" > logs/$log.log 2>&1
    if [ $? != "0" ]; then
        cat logs/$log.log
        echo -e "Some error! Exiting."
        exit 1
    fi
}

#- Wrapper around runme, selecting serial or parallel run
runmePar()
{
    np=$1
    shift 1
    if [ "$np" == "1" ]; then
        runme "$@"
    else
        runme mpirun -np $np "$@" -parallel
    fi
}


setDecomposition()
{
    dict="system/decomposeParDict"
    if [ "$1" == "1" ]; then
        return 0
    elif [ ! -f $dict ]; then
        echo "Error: Missing decomposeParDict"
        exit 1
    fi
    sed -i -e "s%^\s*\(numberOfSubdomains\) .*;%\1 $1;%g" $dict
}

newTempCase()
{
    [[ -d $1 ]] && runme rm -rf $1 ;
    runme mkdir $1;
    ( touch $1/case.foam )
    runme cp -r setup.foam constant system $1/.
}

#- Replaces processor*/0 with serial 0, to keep templates
distributeZero()
{
    [[ -d processor0 ]] || return 0

    if [[ "$1" == "" ]]; then
        sourceDir=0
    else
        sourceDir=$1
    fi

    for p in processor*; do
        rm -rf $p/0
        cp -rv 0 $p/0
    done
}

doTopoRefine()                                                                                                         
{                                                                                                                      
    # refineMesh bugs out in parallel                                                                                  
    for i in $@; do                                                                                                    
        runme topoSet -dict system/topoSetDict.$i                                                                      
        runme refineMesh -dict system/refineMeshDict.$i -overwrite                                                     
    done                                                                                                               
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

