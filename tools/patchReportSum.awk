fmt="%10.4f%10.4f\t%30s\n";
BEGIN{
    Q=0
    V=0
}
{
    if($1 !~ /#Time/){ 
        Q+=$2 
        V+=$3 
        printf(fmt,$2,1000*$3,$1)
    }
}
END { printf(fmt,Q,100*V,"Total") }
