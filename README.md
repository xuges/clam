# clam语言<br>The clam programming language
**CLAM: C Like And More**



## Example

```
int a = 2 * 2 + 4 / 2 + 9 % 2;
int b = a;
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
		d *= 2;
		{
			a = d;
			b++;
		}
	}
	
	c--;
	
	a = first(d, c, b);
	a += 3;
	b -= 1;
	c /= 2;
	d %= 3;
	
	return a;
}
```