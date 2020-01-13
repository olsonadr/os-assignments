#!/bin/bash

### supported functions
# help function
help()
{
    printf "[USAGE]\nmatrix\tdims\t\t[MATRIX]\nmatrix\ttranspose\t[MATRIX]"
    printf "\nmatrix\tmean\t\t[MATRIX]\nmatrix\tadd\t\tMATRIX_LEFT\tMATRIX_RIGHT"
    printf "\nmatrix\tmultiply\tMATRIX_LEFT\tMATRIX_RIGHT\n"
}

# add function
dims()
{
    local rows=0
    local cols=0

    while read myLine
    do
        rows=`expr $rows + 1`

        if [ "$cols" == "0" ]
        then
            while [ ! -z "$myLine" ]
            do
                myLine=${myLine%?}
                cols=`expr $cols + 1`
            done
        fi
    done < $1

    # account for tabs in cols total
    cols=`expr \( $cols + 1 \) / 2`

    # echo result
    echo "$rows"x"$cols"
}

# transpose function
transpose()
{
    row=0

    # read file
    while read myLine
    do
        for digit in $myLine
        do
            echo "$digit" | cat >> "col$row"
        done

        row=`expr $row + 1`
    done < $1

    # decrement row
    row=`expr $row - 1`

    # transpose
    tmpcol=0
    cols=`dims $1`
    cols=${cols#?[x]}

   	while [ "$tmpcol" != "`expr $cols`" ]
   	do
        tmprow=0

   	    while [ "$tmprow" != "`expr $row`" ]
   	    do
            cat "col$tmprow" | tail -n `expr $cols - $tmpcol` | head -n 1 | tr '\n' '\t' >> wow
   	        tmprow=`expr $tmprow + 1`
   	    done

        # last digit of new row
        cat "col$tmprow" | tail -n `expr $cols - $tmpcol` | head -n 1 >> wow
        tmpcol=`expr $tmpcol + 1`
    done

    # output result
    cat wow

    # after transpose
    rm "wow"
    while [ "$row" != "-1" ]
    do
        rm "col$row"
        row=`expr $row - 1`
    done
}

# mean function
mean()
{
    echo mean
}

# add function
add()
{
    if [ "`dims $1`" != "`dims $2`" ]
    then
        echo bad dims
        # non-matching dimensions of args, exit
        return 1
    fi

    rowvar=0

    # read one file into columns from rows
    while read myLine
    do
        for digit in $myLine
        do
            echo "$digit" >> "row$rowvar"
        done
        rowvar=`expr $rowvar + 1`
    done < $1

    rowvar=0

    # fill sum file with each value
    while read myLine
    do
        count=0

        for digit in $myLine
        do
            digit2=$(cat "row$rowvar" | head -n $(expr $count + 1) | tail -n 1)
            digitsum=$(expr $digit2 + $digit)

            printf "$digitsum\t" >> "wow"

            count=$(expr $count + 1)
        done

        printf "\n" >> "wow"
        rowvar=`expr $rowvar + 1`
    done < $2

    # output result
    cat wow

    # remove tmp files
    rm wow
    rm row*
}

# multiply function
multiply()
{
    # Get columns of first
    dims1=`dims $1`
    dims1=${dims1#?[x]}

    # Get rows of second
    dims2=`dims $2`
    dims2=${dims2%[x]?}

    # Check if correct dims for mult
    if [ "$dims1" != "$dims2" ]
    then
        # Error, not correct dims for mult
        echo error
    else
        # Multiply them
        echo multiply
    fi
}


### runtime code

## argument error checking
# check if first arg is a supported operation
if [ "$1" != "help" ] && [ "$1" != "dims" ] && [ "$1" != "transpose" ] \
    && [ "$1" != "mean" ] && [ "$1" != "add" ] && [ "$1" != "multiply" ]
then
    echo unsupported operation / arg error
    # error of unsupported operation or argument error, exit
    exit

# check if right number of args for operation
elif [ "$1" == "dims" ] || [ "$1" == "transpose" ] || [ "$1" == "mean" ]
then
    if [ "$#" != "2" ]
    then
	echo wrong dims
	# error of wrong number of arguments for operation, exit
	exit
    elif [[ ! -r $2 ]]
    then
	echo not readable
	# error of unreadable arguments for operation, exit
	exit
    fi

elif [ "$1" == "add" ] || [ "$1" == "multiply" ]
then
    if [ "$#" != "3" ]
    then
	echo wrong dims
	# error of wrong number of arguments for operation, exit
	exit
    elif [[ ! -r $2 ]] || [[ ! -r $3 ]]
    then
	echo not readable
	# error of unreadable arguments for operation, exit
	exit
    fi
fi

## pass to function listed by first argument
$1 "${@:2}"
