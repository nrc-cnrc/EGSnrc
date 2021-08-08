# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/focal64"
   config.vm.provision "shell", inline: <<-SHELL
     echo "install required packages"
     apt-get update

     # Add desktop environment reference: https://gist.github.com/niw/bed28f823b4ebd2c504285ff99c1b2c2
     apt install -y --no-install-recommends ubuntu-desktop  virtualbox-guest-dkms virtualbox-guest-utils \
        virtualbox-guest-x11
     # Add `vagrant` to Administrator
     usermod -a -G sudo vagrant

     apt-get install -y git gcc g++ gfortran make tk expect \
       libmotif-dev qt5-default imagej
     git clone https://github.com/nrc-cnrc/EGSnrc.git
     cd EGSnrc
     ./HEN_HOUSE/scripts/configure.expect linux.conf 1
     echo "export EGS_HOME=/home/vagrant/EGSnrc/egs_home/" >> /home/vagrant/.bashrc
     echo "export EGS_CONFIG=/home/vagrant/EGSnrc/HEN_HOUSE/specs/linux.conf" >> /home/vagrant/.bashrc
     echo "source /home/vagrant/EGSnrc/HEN_HOUSE/scripts/egsnrc_bashrc_additions" >> /home/vagrant/.bashrc
     source /home/vagrant/.bashrc
     echo $EGS_HOME
     cd $HEN_HOUSE/gui
     make --quiet --print-directory
     cd $HEN_HOUSE/egs++/view/
     make --quiet --print-directory
   SHELL
end
