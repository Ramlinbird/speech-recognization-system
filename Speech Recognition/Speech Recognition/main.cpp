#include "datacapture.h"
#include "featurecomputation.h"

int main(void)
{
	bool status = true;
	if (data_capture())
	{
		cout << "\nCongratulations! You talk is saved in recordData.wav" << endl;
	}
	else
	{
		status = false;
		cout << "\nSory! You talk couldn't be saved in recordData.wav" << endl;
	}
	system("pause");

	if (feature_computation())
	{
		cout << "\nCongratulations! Featrue computation of recordData.wav successed." << endl;
	}
	else
	{
		status = false;
		cout << "\nSorry! Featrue computation of recordData.wav failed." << endl;
	}
	system("pause");

	if (status)
	{
		cout << "\nEverything seems OK so far." << endl;
	}
	else
	{
		cout << "\nSomething wrong in your program." << endl;
	}
	system("pause");

	return 0;
}