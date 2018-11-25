#include <iostream>
#include "my_catalog_manager.cpp"

using std::cout;
using std::endl;
int main()
{
	Catalog_Manager cm;
	cout << cm.table_num << endl;
	cout << cm.tables[0].col_num << endl;
}
 
