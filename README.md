# clam语言<br>The clam programming language
**CLAM: C Like And More**



## Example

```
int a = 1;
int b = 2;
int c = foo();
int d = first(4, 5, 6);

int foo()
{
	return 3;
}

export int first(int arg1, int arg2, int arg3)
{
	return arg1;
}

export int main()
{
	{
		int d = 8;
		{
			a = d;
		}
	}
	
	a = first(d, c, b);
	a = b = c = d = 8;
	
	return a;
}
```