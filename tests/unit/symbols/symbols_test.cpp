#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/Symbols.h"

int errors = 0;

void append(Symbols &symbols, const char *name, uint32_t address)
{
  if (symbols.append(name, address) != 0)
  {
    printf("Error: %s %d  %s:%d\n", name, address, __FILE__, __LINE__);
    errors++;
  }
}

void check_symbols_count(Symbols &symbols, int count)
{
  if (symbols.count() != count)
  {
    printf("Error: symbols_count != %d  %s:%d\n", count, __FILE__, __LINE__);
    errors++;
  }
}

void check_lookup(
  Symbols &symbols,
  const char *name,
  uint32_t expected,
  int expected_ret)
{
  uint32_t address = 0;

  int ret = symbols.lookup(name, &address);

  if (ret != expected_ret || address != expected)
  {
    printf("Error: %s != %d (%d) ret=%d %s:%d\n",
      name,
      expected,
      address,
      ret,
      __FILE__,
      __LINE__);
    errors++;
  }
}

void check_export(Symbols &symbols, const char *name, int expected)
{
  int ret = symbols.export_symbol(name);

  if (ret != expected)
  {
    printf("Error: %s export %d (%d) %s:%d\n",
      name,
      expected,
      ret,
      __FILE__,
      __LINE__);
    errors++;
  }
}

int main(int argc, char *argv[])
{
  Symbols symbols;

  append(symbols, "test1", 100);
  append(symbols, "test2", 200);
  append(symbols, "test3", 300);

  check_symbols_count(symbols, 3);

  check_lookup(symbols, "test1", 100, 0);
  check_lookup(symbols, "test2", 200, 0);
  check_lookup(symbols, "test3", 300, 0);
  check_lookup(symbols, "nothing", 0, -1);

  // If the symbol was never created with symbols_set() it shouldn't
  // change here.
  symbols.set("test1", 150);
  check_lookup(symbols, "test1", 100, 0);

  // This symbol should change.
  symbols.set("test4", 150);
  check_lookup(symbols, "test4", 150, 0);
  symbols.set( "test4", 100);
  check_lookup(symbols, "test4", 100, 0);

  symbols.scope_start();
  append(symbols, "test5", 333);
  check_lookup(symbols, "test5", 333, 0);
  append(symbols, "test4", 444);
  check_lookup(symbols, "test4", 444, 0);
  symbols.scope_end();

  // Check test5 out of scope
  check_lookup(symbols, "test5", 0, -1);

  // Test to make sure global test4 didn't change
  check_lookup(symbols, "test4", 100, 0);

  // Enter another scope
  symbols.scope_start();
  check_lookup(symbols, "test5", 0, -1);
  append(symbols, "test5", 1000);
  check_lookup(symbols, "test5", 1000, 0);
  append(symbols, "test4", 2000);
  check_lookup(symbols, "test4", 2000, 0);
  check_export(symbols, "test4", -1);
  symbols.scope_end();

  // Check test5 out of scope
  check_lookup(symbols, "test5", 0, -1);

  // Test to make sure global test4 didn't change
  check_lookup(symbols, "test4", 100, 0);

  // From a global context, test4 should export (and test1)
  check_export(symbols, "test4", 0);
  check_export(symbols, "test1", 0);

  symbols.lock();
  append(symbols, "test4", 50);
  check_symbols_count(symbols, 8);

  if (errors != 0)
  {
    symbols.print(stdout);
  }

  printf("Total errors: %d\n", errors);
  printf("%s\n", errors == 0 ? "PASSED." : "FAILED.");

  if (errors != 0) { return -1; }

  return 0;
}

