#ifndef ARRAY_H
#define ARRAY_H

enum ARRAY_STATE {
	STATIC,
	SORTING,
	LIFT_SELECTED_VALUE,
	SLIDE_SELECTED_VALUE,
	DROP_SELECTED_VALUE,
	COPY_COMPARE_VALUE
};

typedef struct ArrayStruct {
	ARRAY_STATE state;
	
	int selected_value_index;
	int compare_value_index;
	int selected_value;
	double selected_value_x_shift;
	double selected_value_y_shift;
	double compare_value_x_shift;

	int size;
	int* array;
} ArrayStruct;

void InsertionSort(int* array, int size) {
	int i;
	int val;
	int j;
	for(i = 1; i < size; ++i) {
		j = i - 1;
		val = array[i];

		while((0 <= j) && (val < array[j])) {
			array[j + 1] = array[j];
			j -= 1;
		}
		array[j + 1] = val;
	}
}

#endif
