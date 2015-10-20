#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdbool.h>

#define MAX_LEVEL 5	//最大层数，可根据数据规模进行调整，一般为log(n)

//跳表节点
typedef struct skipNode
{
	int key;
	int value;
	struct skipNode *next[1];	//柔性数组,这里可以为0，但是让其为1是为了调试时更好的查看第0层的列表
}SkipNode;

//跳表
typedef struct skiplist
{
	int level;
	SkipNode *head;

}SkipList;



//创建跳表节点
SkipNode* createNode(int level, int key, int value)
{
	//通过柔性数组，得到包含level层的SkipNode*类型的存储空间
	SkipNode *p = (SkipNode*)malloc(sizeof(SkipNode) + level*sizeof(SkipNode*));
	if (!p)
		return NULL;
	p->key = key;
	p->value = value;

	return p;
}

//创建跳表
SkipList* createSkipList(int key, int value)
{
	SkipList *sl = (SkipList*)malloc(sizeof(SkipList));
	if (!sl)
		return NULL;

	sl->level = 1;
	SkipNode *h = createNode(MAX_LEVEL, key, value);
	if (!h)
	{
		free(sl);
		return NULL;
	}

	sl->head = h;
	for (int i = 0; i < MAX_LEVEL; ++i)
	{
		h->next[i] = NULL;
	}

	//初始化随机数种子
	srand(time(0));

	return sl;
}

//随机产生一个层数
int randomLevel()
{
	int level = 1;
	while (rand() % 2 == 1)
	{
		level++;
	}

	level = level < MAX_LEVEL ? level : MAX_LEVEL;

	return level;
}

bool insert(SkipList *sl, int key, int value)
{
	if (!sl)
		return false;

	SkipNode *update[MAX_LEVEL];	//保存那些需要更新的节点
	SkipNode *p = sl->head;
	SkipNode *q = NULL;

	//第一步：找出需要更新的MAX_LEVEl个节点。如果一个节点的下一个节点为NULL或者下一个节点的值大于新插入节点，那么其就需要被更新
	int i = sl->level-1;
	for ( ; i >= 0; --i)
	{
		//在同一层中找到需要更新的节点
		while ((q = p->next[i]) && q->key < key)
		{
			p = q;
		}
		update[i] = p;
	}

	//第二步：随机生成新的层数，并处理
	int level = randomLevel();
	if (level > sl->level)
	{
		//有可能level要超过sl->level很多，但是没有必要全部满足，只需要增加一层即可,后面的事情谁说的准呢，先做好当前的事情即可。
		sl->level++;
		level = sl->level;
		update[level - 1] = sl->head;
	}

	//第三步：更新Update数组
	q = createNode(level, key, value);	//注意：这里不好处理新插入节点比头节点还小的情况，所以可以设置一个很小数字为冗余的头节点
	for (i = level - 1; i >= 0; --i)	//超过level层，新增的节点根本触碰不到，无需更新
	{
		p = update[i];	//类似单链表插入节点的更新
		q->next[i] = p->next[i];
		p->next[i] = q;
	}

	return true;
}

bool delete(SkipList *sl, int key)
{
	if (!sl)
		return false;

	SkipNode *update[MAX_LEVEL];	//保存可能需要更新的节点
	SkipNode *p = sl->head;
	SkipNode *q = NULL;
	int i = 0;
	for (i = sl->level - 1; i >= 0; --i)
	{
		while ((q = p->next[i]) && q->key < key)
		{
			p = q;
		}
		update[i] = p;
	}
	//如果存在key节点，那么最终q肯定就是它
	if (!q || q->key != key)
	{
		return false;	//需要删除的节点不存在
	}

	for (i = sl->level - 1; i >= 0; --i)
	{
		p = update[i];
		if (p->next[i] == q)	//因为q的level可能很低，上层的update节点可以不用管
		{
			p->next[i] = q->next[i];
			if (sl->head->next[i] == NULL)	//如果删除的是最高层的节点，也就是目前删除后只剩head节点，那么可以进行优化。
			{
				sl->level--;
			}
		}
	}

	free(q);
	q = NULL;

	return true;
}

bool find(SkipList *sl, int key)
{
	if (!sl)
		return false;

	int i;
	SkipNode *p = sl->head;
	SkipNode *q = NULL;
	for (i = sl->level - 1; i >= 0; --i)
	{
		while ((q = p->next[i]) && q->key < key)
		{
			p = q;
		}
		if (q && q->key == key)
			return true;
	}

	return false;
}

//按层次打印跳表
void print(SkipList *sl)
{
	if (!sl)
		return;

	SkipNode *p = NULL;
	int i = 0;
	for (i = sl->level - 1; i >= 0; --i)
	{
		p = sl->head;
		while (p)
		{
			printf("%d ", p->key);
			p = p->next[i];
		}
		printf("\n");
	}
}

void destroy(SkipList *sl)
{
	//销毁的关键是要删除所有的节点，跳表的一个特点就是第0层包含了所有的节点
	if (!sl)
		return;

	SkipNode *p = sl->head;
	SkipNode *q = NULL;
	while (!p)
	{
		q = p;
		p = p->next[0];
		free(q);
	}

	free(sl);
	sl = NULL;
}

int main(void)
{
	int i;
	SkipList *sl = createSkipList(-1, -1);	//设置头节点的值

	for (i = 0; i < 23; ++i)
	{
		insert(sl, i, i*i);
	}

	print(sl);

	destroy(sl);

	getchar();

	return 0;
}
