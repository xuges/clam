# clam语言<br>The clam programming language
**CLAM: C Like And More**



## Example

```
int num = 2 * 2 + 4 / 2 - 9 % 2;
bool odd = (num + 1) % 2 == 0;

export int main()
{
	int a = odd ? 1 : 2;
	bool one = a & 1 == 1;
	int b = a | 1;
	int c = b ^ 1;
	c = ~c;
	int d = 1 << 2;
	d >>= 1;
	
	if (one && b == 1 || c != 0)
		return 1;
	else
		return 2;
		
	return 0;
}
```

