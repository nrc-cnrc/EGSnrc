:start geometry definition:

    :start geometry:
        name = myxyz_labeled
        library = egs_ndgeometry
        type = EGS_XYZGeometry
        x-planes = -7, -6, -5
        y-planes = -7, -6
        z-planes = -1, 1
        set label = left_voxel 0
        set label = right_voxel 1
        :start media input:
            media = water
        :stop media input:
    :stop geometry:

    simulation geometry = myxyz_labeled

:stop geometry definition:
