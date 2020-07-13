#!/bin/bash
input="nuovi_punteggi_non_valutati.csv"
while IFS= read -r line
do
    echo "$line" >tmp.csv
    
    r=$(./rns -c -f tmp.csv -d 2 20 2 -n 100  -l1 layer1.w -l2 layer2.w)

    if [ "$r" = "1" ]; then
        echo "$line,AMMESSO"
    else
        echo "$line,NON AMMESSO"
    fi
    
    
done < "$input"
