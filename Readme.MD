# KansGL

> KansGL是 一个opengl的学习demo,他的部分架构基于Kans3D,我所编写的一个渲染器，如果你对Kans3D感兴趣可以点击链接
>
> [Kans3D](https://github.com/kanzakishigure/Kans3D)
>
> > 不过比较遗憾的是Kans3D的开源仓库已经有较长一段时间没有更新了，因为我正在重写老的的渲染管线，并逐步以Vulkan代替Opengl.
>
> 演示效果
>
> ![图片](https://cdn.staticaly.com/gh/KanShigure/image_repository@main/20221219/图片.44gchzkzry68.webp)
>
> ### 如何构建
>
> 运行script文件夹下的 win GenProject.bat，就能快速生成解决方案，如果你使用的不是vs2022，请将 win GenProject.bat 修改为
>
> ```bash
> pushd ..\
> call  vendor\bin\premake\premake5.exe vs2019
> 
> popd
> pause
> ```
>
> 如果你想生成其他解决方案，请查看premake的 [文档](https://premake.github.io/)
