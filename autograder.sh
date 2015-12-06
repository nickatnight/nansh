#!/bin/bash

##########################################################
#   Instructions:
#     download file into folder 'Two'
#     cd into your 'Two' folder and execute these commands:
#     cp -r ~cs570/Data4/ Data4/
#     ./autograder.sh
##########################################################

rm your.*
make clean
make

FILES=("Data4/"*)

for f in "${FILES[@]}"
do
    echo "Processing ${f} file..."
    echo "-"

    cat "${f}"
    echo ""
    echo "-------------"
    echo "Your p2's out:"
    echo "-------------"
    ./nansh < "${f}"

    echo ""
    echo ""

done

echo "############################"
echo "#       Output Files       #"
echo "############################"
echo ""
OUTS=("your.output"*)

for o in "${OUTS[@]}"
do
    echo "File Name: ${o}"
    echo "Contents:"
    cat "${o}"
    echo "-"
done


# http://omus.sdsu.edu
