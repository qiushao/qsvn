### 1. 用beyondcompare来diff SVN中的代码
在目录~/.subversions/下，新建脚本文件 mydiff.sh, 内容如下： 

```
#!/bin/sh

# Configure your favorite diff program here.
DIFF="/usr/bin/bcompare"
# DIFF="/usr/bin/meld"
# DIFF="/usr/bin/kompare"

# Subversion provides the paths we need as the sixth and seventh
# parameters.
LEFT=${6}
RIGHT=${7}

# Call the diff command (change the following line to make sense for
# your merge program).
$DIFF $LEFT $RIGHT

# Return an errorcode of 0 if no differences were detected, 1 if some were.
# Any other errorcode will be treated as fatal.
return 0
```

并给其添加可执行权限.
```
chmod 777 ~/.subversion/mydiff.sh
```

修改 ~/.subversions/config 文件, 查找 diff-cmd， 找注释去掉，并修改如下
```
diff-cmd = ~/.subversion/mydiff.sh
```



### 2. 设置右键菜单


