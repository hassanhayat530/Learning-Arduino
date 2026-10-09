#include <stdio.h>
#include "temps.h" // this file defines temperatures[] and no_value constants

constexpr int list_size = sizeof(temperatures) / sizeof(temperatures[0]); // specifying size of temperatures list

// Function to find the minimal measured temperature
int find_minimal(const int temperatures_list[], int size, int no_val) {
    int minimal = 0;

    for (int i = 0; i < size; ++i) {
        // Update smallest if current element is smaller
        if ((temperatures_list[i] < minimal) && (temperatures_list[i] != no_val)) {
            minimal = temperatures_list[i];
        }
    }
    return minimal;
}

// Function to print a certain number of symbols
void print_symbols(int count, char symbol) {
    for (int i = 0; i < count; ++i) {
        printf("%c", symbol);
    }
}

// Function to conditionally print symbols
void print_line(int temperature, int minimal) {

    // Calculate indentation based on the minimum temperature
    int indentation = 0;
    if (minimal < 0)
        indentation = -minimal;

    // Calculate temperature indentation
    int temperature_indentation = 0;
    if (temperature < 0)
        temperature_indentation = -temperature;

    indentation = indentation - temperature_indentation;

    print_symbols(indentation, ' '); // Print the indentation

    if (temperature < 0) {
        print_symbols(-temperature, '*');
        printf("|\n");
    } else {
        printf("|");
        print_symbols(temperature, '*');
        printf("\n");
    }
}


// Printing the thermometer
void print_thermometer(const int temperatures_list[], int size, int no_val) {
    int minimal = find_minimal(temperatures_list, size, no_val); // declaring value of smallest temperature

    int prev_value = 0; // handling the case where first value is no_value

    for (int i = 0; i < size; ++i) {
        int temperature = temperatures_list[i];

        // handling the case with missing values
        if (temperatures_list[i] == no_val) {
            temperature = prev_value;
        } else {
            prev_value = temperature;
        }

        print_line(temperature, minimal);
    }
}

// main function
int main(int argc, char **argv)
{
    // Printing the thermometer
    print_thermometer(temperatures, list_size, no_value);

    return 0;
}
