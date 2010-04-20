package org.scourge.model;

import com.jme.math.Vector3f;
import org.scourge.config.Pc;
import org.scourge.terrain.Md2Model;
import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Element;

/**
 * User: gabor
 * Date: Apr 17, 2010
 * Time: 8:12:54 PM
 */
public class Creature {
    @Attribute(name = "version")
    private int version;

    @Element(name = "name")
    private String name;

    @Element(name = "model")
    private String model;

    @Element(name = "skin")
    private String skin;

    @Element(name = "position")
    private float[] position;

    @Element(name = "level")
    private int level;

    @Element(name = "experience")
    private long experience;

    @Element(name = "hp")
    private int hp;

    @Element(name = "mp")
    private int mp;

    @Element(name = "coins")
    private int coins;

    @Element(name = "sex")
    private int sex;

    @Element(name = "portrait")
    private String portrait;


    // not saved
    private Md2Model creatureModel;

    public Creature() {
        version = 1;
        //693, 9, 151);
        //389, 9, 349);
        //498, 9, 489);
        position = new float[] { 651, 9, 413 };
        name = "zorro";
        model = "./data/models/sfod8/tris.md2";
        skin = "./data/models/sfod8/Rieger.png";
        level = 1;
        experience = 0;
        hp = 10;
        mp = 8;
        coins = (int)(Math.random() * 5) + 3;
        sex = Pc.Sex.male.ordinal();
        portrait = Pc.PORTRAIT[sex][0];

        // init the model
        creatureModel = new Md2Model(model, skin);
        creatureModel.setKeyFrame(Md2Model.Md2Key.stand);

        afterLoad();
    }

    public void beforeSave() {
        setPosition(new float[] {
                creatureModel.getX(), 9, creatureModel.getZ()
        });
    }

    public void afterLoad() {
        creatureModel.moveTo(new Vector3f(position[0], position[1], position[2]));
    }

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getModel() {
        return model;
    }

    public void setModel(String model) {
        this.model = model;
    }

    public String getSkin() {
        return skin;
    }

    public void setSkin(String skin) {
        this.skin = skin;
    }

    public float[] getPosition() {
        return position;
    }

    public void setPosition(float[] position) {
        this.position = position;
    }

    public Md2Model getCreatureModel() {
        return creatureModel;
    }

    public void setCreatureModel(Md2Model creatureModel) {
        this.creatureModel = creatureModel;
    }

    public int getLevel() {
        return level;
    }

    public void setLevel(int level) {
        this.level = level;
    }

    public long getExperience() {
        return experience;
    }

    public void setExperience(long experience) {
        this.experience = experience;
    }

    public int getHp() {
        return hp;
    }

    public void setHp(int hp) {
        this.hp = hp;
    }

    public int getMp() {
        return mp;
    }

    public void setMp(int mp) {
        this.mp = mp;
    }

    public int getCoins() {
        return coins;
    }

    public void setCoins(int coins) {
        this.coins = coins;
    }

    public int getSex() {
        return sex;
    }

    public void setSex(int sex) {
        this.sex = sex;
    }

    public String getPortrait() {
        return portrait;
    }

    public void setPortrait(String portrait) {
        this.portrait = portrait;
    }
}
