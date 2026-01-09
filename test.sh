#!/bin/bash
#compare nm with ft_nm on the file that are in test/

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

for file in test/*; do
    echo -n "Comparing $file... "
    # Use LC_ALL=C to enforce ASCII sorting for nm to match ft_nm/strcmp behavior
    # Redirect stderr to /dev/null to ignore errors on invalid files
    # Filter out "bfd plugin" messages which oddly appear on stdout on some systems
    LC_ALL=C nm $file 2>/dev/null | grep -v "bfd plugin" > nm_output.txt
    ./ft_nm $file > ft_nm_output.txt 2>/dev/null
    
    DIFF=$(diff nm_output.txt ft_nm_output.txt)
    if [ "$DIFF" != "" ]; then
        echo -e "${RED}KO${NC}"
        echo "$DIFF"
    else
        echo -e "${GREEN}OK${NC}"
    fi
    rm nm_output.txt ft_nm_output.txt
done

echo -e "\n\nAll tests completed. Now doing it with bonus options."

bonus_flags=("-r" "-a" "-g" "-p" "-u")
for file in test/*; do
    echo -n "Comparing $file... "
    for flags in "${bonus_flags[@]}"; do
        LC_ALL=C nm "$flags" "$file" 2>/dev/null | grep -v "bfd plugin" > nm_output.txt
        ./ft_nm "$flags" "$file" > ft_nm_output.txt 2>/dev/null

        DIFF=$(diff nm_output.txt ft_nm_output.txt)
        if [ "$DIFF" != "" ]; then
            echo -e "${RED}KO${NC} with flag $flags"
            echo "$DIFF"
        else
            echo -e "${GREEN}OK${NC}"
        fi
        rm nm_output.txt ft_nm_output.txt
    done
done
echo -e "\n\nAll bonus tests completed."
