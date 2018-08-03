#include "fx_manager.h"

#include "fx_color.h"
#include "fx_emitter_def.h"
#include "fx_particle_def.h"
#include "fx_particle_system.h"
#include "fx_rect.h"

namespace fx {

static const FVec2 dir_vecs[8] = {{0.0, -1.0}, {0.0, 1.0},   {1.0, 0.0}, {-1.0, 0.0},
                                  {1.0, -1.0}, {-1.0, -1.0}, {1.0, 1.0}, {-1.0, 1.0}};
FVec2 dirToVec(Dir dir) { return dir_vecs[(int)dir]; }

using SubSystemDef = ParticleSystemDef::SubSystem;

static void addTestSimpleEffect(FXManager &mgr) {
  EmitterDef edef;
  edef.strengthMin = edef.strengthMax = 30.0f;
  edef.frequency = {{10.0f, 55.0f, 0.0f, 0.0}, InterpType::cosine};

  ParticleDef pdef;
  pdef.life = 1.0f;
  pdef.size = 32.0f;
  pdef.alpha = {{0.0f, 0.1, 0.8f, 1.0f}, {0.0, 1.0, 1.0, 0.0}, InterpType::linear};

  pdef.color = {{{1.0f, 1.0f, 0.0f}, {0.5f, 1.0f, 0.5f}, {0.2f, 0.5f, 1.0f}}, InterpType::linear};
  pdef.textureName = "circular.png";

  ParticleSystemDef psdef;
  psdef.subSystems.emplace_back(mgr.addDef(pdef), mgr.addDef(edef), 0.0f, 5.0f);
  psdef.name = "test_simple";
  mgr.addDef(psdef);
}

static void addTestMultiEffect(FXManager &mgr) {
  EmitterDef edef;
  edef.strengthMin = edef.strengthMax = 30.0f;
  edef.frequency = {{10.0f, 55.0f, 0.0f, 0.0}, InterpType::cosine};

  FVec3 colors[5] = {{0.7f, 0.2, 0.2f}, {0.2f, 0.7f, 0.2f}, {0.2f, 0.2f, 0.7f}, {0.9f, 0.2, 0.9f}, {0.3f, 0.9f, 0.4f}};

  ParticleDef pdefs[5];
  for (int n = 0; n < 5; n++) {
    pdefs[n].life = 1.0f;
    pdefs[n].size = 32.0f;
    pdefs[n].alpha = {{0.0f, 0.1, 0.8f, 1.0f}, {0.0, 1.0, 1.0, 0.0}, InterpType::linear};
    pdefs[n].color = {colors[n]};
    pdefs[n].textureName = "circular.png";
  }

  ParticleSystemDef psdef;
  psdef.name = "test_multi";
  for (int n = 0; n < 5; n++)
    psdef.subSystems.emplace_back(mgr.addDef(pdefs[n]), mgr.addDef(edef), float(n) * 0.5f, float(n) * 1.5f + 2.0f);
  mgr.addDef(psdef);
}

static void addWoodSplinters(FXManager &mgr) {
  EmitterDef edef;
  edef.strengthMin = 20.0f;
  edef.strengthMax = 60.0f;
  edef.rotationSpeedMin = -0.5f;
  edef.rotationSpeedMax = 0.5f;
  edef.frequency = 999.0f;

  ParticleDef pdef;
  pdef.life = 1.0f;
  pdef.size = 4.0f;
  pdef.slowdown = {{0.0f, 0.1f}, {5.0f, 1000.0f}};
  pdef.alpha = {{0.0f, 0.3f, 1.0f}, {1.0, 1.0, 0.0}, InterpType::cosine};

  auto animateFunc = [](AnimationContext &ctx, Particle &pinst) {
    defaultAnimateParticle(ctx, pinst);
    float shadow_min = 5.0f, shadow_max = 10.0f;
    if (pinst.pos.y < shadow_min) {
      float dist = shadow_min - pinst.pos.y;
      pinst.movement += FVec2(0.0f, dist);
    }
    pinst.pos.y = min(pinst.pos.y, shadow_max);
  };

  FColor brown(IColor(120, 87, 46));
  // Kiedy cząsteczki opadną pod drzewo, robią się w zasięgu cienia
  // TODO: lepiej rysować je po prostu pod cieniem
  pdef.color = {{0.0f, 0.15f, 0.17}, {brown.rgb(), brown.rgb(), brown.rgb() * 0.6f}};
  pdef.textureName = "flakes_4x4_borders.png";
  pdef.textureTiles = {4, 4};

  SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef), 0.0f, 0.1f);
  ssdef.maxTotalParticles = 7;
  ssdef.animateFunc = animateFunc;

  ParticleSystemDef psdef;
  psdef.subSystems = {ssdef};
  psdef.name = "wood_splinters";
  mgr.addDef(psdef);
}

static void addRockSplinters(FXManager &mgr) {
  // Opcja: spawnowanie splinterów na tym samym kaflu co minion:
  // - Problem: Te particle powinny się spawnować na tym samym tile-u co imp
  //   i spadać mu pod nogi, tak jak drewnianie drzazgi; Tylko jak to
  //   zrobić w przypadku kafli po diagonalach ?
  // - Problem: czy te particle wyświetlają się nad czy pod impem?
  //
  // Chyba prościej jest po prostu wyświetlać te particle na kaflu z rozwalanym
  // murem; Zresztą jest to bardziej spójne z particlami dla drzew
  EmitterDef edef;
  edef.strengthMin = 20.0f;
  edef.strengthMax = 60.0f;
  edef.rotationSpeedMin = -0.5f;
  edef.rotationSpeedMax = 0.5f;
  edef.frequency = 999.0f;

  ParticleDef pdef;
  pdef.life = 1.0f;
  pdef.size = 4.0f;
  pdef.slowdown = {{0.0f, 0.1f}, {5.0f, 1000.0f}};
  pdef.alpha = {{0.0f, 0.4f, 1.0f}, {1.0, 1.0, 0.0}, InterpType::cosine};

  pdef.color = FVec3(0.4, 0.4, 0.4);
  pdef.textureName = "flakes_4x4_borders.png";
  pdef.textureTiles = {4, 4};

  SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef), 0.0f, 0.1f);
  ssdef.maxTotalParticles = 5;

  ParticleSystemDef psdef;
  psdef.subSystems = {ssdef};
  // TODO: cząsteczki mogą mieć różny czas życia
  psdef.name = "rock_splinters";
  mgr.addDef(psdef);
}

static void addRockCloud(FXManager &mgr) {
  // Spawnujemy kilka chmurek w ramach kafla;
  // mogą być większe lub mniejsze
  //
  // czy zostawiają po sobie jakieś ślady?
  // może niech zostają ślady po splinterach, ale po chmurach nie?
  EmitterDef edef;
  edef.source = FRect(-5.0f, -5.0f, 5.0f, 5.0f);
  edef.strengthMin = 5.0f;
  edef.strengthMax = 8.0f;
  edef.frequency = 60.0f;

  ParticleDef pdef;
  pdef.life = 3.5f;
  pdef.size = {{0.0f, 0.1f, 1.0f}, {15.0f, 30.0f, 38.0f}};
  pdef.alpha = {{0.0f, 0.05f, 0.2f, 1.0f}, {0.0f, 0.3f, 0.4f, 0.0f}};
  pdef.slowdown = {{0.0f, 0.2f}, {0.0f, 10.0f}};

  FVec3 start_color(0.6), end_color(0.4);
  pdef.color = {{start_color, end_color}};
  pdef.textureName = "clouds_soft_4x4.png";
  pdef.textureTiles = {4, 4};

  SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef), 0.0f, 0.1f);

  // TODO: różna liczba początkowych cząsteczek
  ssdef.maxTotalParticles = 5;

  ParticleSystemDef psdef;
  psdef.subSystems = {ssdef};
  psdef.name = "rock_clouds";
  mgr.addDef(psdef);
}

static void addExplosionEffect(FXManager &mgr) {
  // TODO: tutaj trzeba zrobić tak, żeby cząsteczki które spawnują się później
  // zaczynały z innym kolorem
  EmitterDef edef;
  edef.strengthMin = edef.strengthMax = 15.0f;
  edef.frequency = 60.0f;

  ParticleDef pdef;
  pdef.life = 0.5f;
  pdef.size = {{5.0f, 30.0f}};
  pdef.alpha = {{0.0f, 0.5f, 1.0f}, {0.3, 0.4, 0.0}};

  IColor start_color(255, 244, 88), end_color(225, 92, 19);
  pdef.color = {{FColor(start_color).rgb(), FColor(end_color).rgb()}};
  pdef.textureName = "clouds_soft_borders_4x4.png";
  pdef.textureTiles = {4, 4};

  SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef), 0.0f, 0.5f);
  ssdef.maxTotalParticles = 20;

  ParticleSystemDef psdef;
  psdef.subSystems = {ssdef};
  psdef.name = "explosion";
  mgr.addDef(psdef);
}

static void addRippleEffect(FXManager &mgr) {
  EmitterDef edef;

  // Ta animacja nie ma sprecyzowanej długości;
  // Zamiast tego może być włączona / wyłączona albo może się zwięszyć/zmniejszyć jej siła
  // krzywe które zależą od czasu animacji tracą sens;
  // animacja może być po prostu zapętlona
  edef.frequency = 1.5f;
  edef.initialSpawnCount = 1.0f;

  // First scalar parameter controls how fast the ripples move
  auto prepFunc = [](AnimationContext &ctx, EmissionState &em) {
    float freq = defaultPrepareEmission(ctx, em);
    float mod = 1.0f + ctx.ps.params.scalar[0];
    return freq * mod;
  };

  auto animateFunc = [](AnimationContext &ctx, Particle &pinst) {
    float ptime = pinst.particleTime();
    float mod = 1.0f + ctx.ps.params.scalar[0];
    float timeDelta = ctx.timeDelta * mod;
    pinst.pos += pinst.movement * timeDelta;
    pinst.rot += pinst.rotSpeed * timeDelta;
    pinst.life += timeDelta;
  };

  ParticleDef pdef;
  pdef.life = 1.5f;
  pdef.size = {{10.0f, 50.0f}};
  pdef.alpha = {{0.0f, 0.3f, 0.6f, 1.0f}, {0.0f, 0.3f, 0.5f, 0.0f}};

  pdef.color = FVec3(1.0f, 1.0f, 1.0f);
  pdef.textureName = "torus.png";

  // TODO: możliwośc nie podawania czasu emisji (etedy emituje przez całą długośc animacji)?
  SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef), 0.0f, 1.0f);
  ssdef.maxActiveParticles = 10;
  ssdef.prepareFunc = prepFunc;
  ssdef.animateFunc = animateFunc;

  ParticleSystemDef psdef;
  psdef.subSystems = {ssdef};
  psdef.animLength = 1.0f;
  psdef.isLooped = true;
  psdef.name = "ripple";

  mgr.addDef(psdef);
}

static void addCircularBlast(FXManager &mgr) {
  EmitterDef edef;
  edef.frequency = 50.0f;
  edef.initialSpawnCount = 10.0;

  auto prepFunc = [](AnimationContext &ctx, EmissionState &em) {
    defaultPrepareEmission(ctx, em);
    return 0.0f;
  };

  auto emitFunc = [](AnimationContext &ctx, EmissionState &em, Particle &new_inst) {
    new_inst.life = min(em.maxLife, float(ctx.ss.totalParticles) * 0.01f);
    new_inst.maxLife = em.maxLife;
  };

  auto animateFunc = [](AnimationContext &ctx, Particle &pinst) {
    float ptime = pinst.particleTime();
    float mod = 1.0f + ctx.ps.params.scalar[0];
    float timeDelta = ctx.timeDelta * mod;
    pinst.life += pinst.movement.x;
    pinst.movement.x = 0;
    pinst.life += timeDelta;
  };

  ParticleDef pdef;
  pdef.life = 0.5f;
  pdef.size = {{10.0f, 80.0f}, InterpType::cosine};
  pdef.alpha = {{0.0f, 0.03f, 0.2f, 1.0f}, {0.0f, 0.15f, 0.15f, 0.0f}, InterpType::cosine};

  pdef.color = {{0.5f, 0.8f}, {{1.0f, 1.0f, 1.0f}, {0.5f, 0.5f, 1.0f}}};
  pdef.textureName = "torus.png";

  SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef), 0.0f, 0.1f);
  ssdef.maxActiveParticles = 20;
  ssdef.prepareFunc = prepFunc;
  ssdef.animateFunc = animateFunc;
  ssdef.emitFunc = emitFunc;

  ParticleSystemDef psdef;
  psdef.subSystems = {ssdef};
  psdef.name = "circular_blast";

  mgr.addDef(psdef);
}

static void addFeetDustEffect(FXManager &mgr) {
  // Ten efekt musi zaznaczać fakt, że postać się porusza w jakimś kierunku
  // To musi byc kontrolowalne za pomocą parametru
  // FX jest odpalany w momencie gdy postać wychodzi z kafla ?
  //
  // TODO: Parametrem efektu powinien być enum: kierunek ruchu
  // zależnie od tego mamy różne kierunki generacji i inny punkt startowy
  // drugim parametrem jest kolor (choć chyba będzie uzywany tylko na piasku?)

  EmitterDef edef;
  edef.source = FRect(-3, 3, 3, 4);
  edef.strengthMin = 15.0f;
  edef.strengthMax = 20.0f;
  edef.frequency = 60.0f;

  ParticleDef pdef;
  pdef.life = 1.25f;
  pdef.size = {{0.0f, 0.1f, 1.0f}, {5.0f, 14.0f, 20.0f}, InterpType::quadratic};
  pdef.alpha = {{0.0f, 0.05f, 0.2f, 1.0f}, {0.0f, 0.2f, 0.3f, 0.0f}};
  pdef.slowdown = {{0.0f, 0.2f}, {0.0f, 10.0f}};

  FVec3 start_color(0.9), end_color(0.7);
  pdef.color = {{start_color, end_color}};
  pdef.textureName = "clouds_soft_4x4.png";
  pdef.textureTiles = {4, 4};

  SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef), 0.0f, 0.2f);
  // TODO: różna liczba początkowych cząsteczek
  ssdef.maxTotalParticles = 3;

  ssdef.emitFunc = [](AnimationContext &ctx, EmissionState &em, Particle &pinst) {
    auto dvec = dirToVec(ctx.ps.params.dir[0]);
    defaultEmitParticle(ctx, em, pinst);
    pinst.pos -= dvec * 4.0f;
    pinst.rot = 0.0f;
    pinst.size = FVec2(1.2f, 0.6f);
  };
  ssdef.prepareFunc = [](AnimationContext &ctx, EmissionState &em) {
    auto ret = defaultPrepareEmission(ctx, em);
    auto vec = normalize(dirToVec(ctx.ps.params.dir[0]));
    em.angle = vectorToAngle(vec);
    em.angleSpread = 0.0f;
    return ret;
  };

  ParticleSystemDef psdef;
  psdef.subSystems = {ssdef};
  psdef.name = "feet_dust";
  mgr.addDef(psdef);
}

static void addMagicMissleEffect(FXManager &mgr) {
  // Każda cząsteczka z czasem z grubsza liniowo przechodzi od źródła do celu
  // dodatkowo może być delikatnie przesunięta z głównego toru

  ParticleSystemDef psdef;
  { // Base system, not visible, only a source for other particles
    EmitterDef edef;
    edef.strengthMin = edef.strengthMax = 100.0f;
    edef.frequency = 60.0f;
    edef.angleSpread = fconstant::pi;

    ParticleDef pdef;
    pdef.life = .45f;
    pdef.size = {{15.0f, 20.0f}};
    pdef.alpha = 0.0f; // TODO: don't draw invisible particles
    pdef.slowdown = 1.0f;

    SubSystemDef ssdef_base(mgr.addDef(pdef), mgr.addDef(edef), 0.0f, 0.5f);
    ssdef_base.maxTotalParticles = 1;

    ssdef_base.animateFunc = [](AnimationContext &ctx, Particle &pinst) {
      defaultAnimateParticle(ctx, pinst);
      float attract_value = max(0.0001f, std::pow(min(1.0f, 1.0f - pinst.particleTime()), 5.0f));
      float mul = std::pow(0.001f * attract_value, ctx.timeDelta);
      pinst.pos *= mul;
    };

    ssdef_base.drawFunc = [](DrawContext &ctx, const Particle &pinst, DrawParticle &out) {
      Particle temp(pinst);
      temp.pos += temp.particleTime() * ctx.ps.targetOff;
      defaultDrawParticle(ctx, temp, out);
    };
    psdef.subSystems.emplace_back(ssdef_base);
  }

  { // Secondary system
    EmitterDef edef;
    edef.strengthMin = edef.strengthMax = 40.0f;
    edef.frequency = 50.0f;
    edef.angleSpread = fconstant::pi;

    ParticleDef pdef;
    pdef.life = .3f;
    pdef.size = {{20.0f, 25.0f}};
    pdef.alpha = {{0.0f, 0.5f, 1.0f}, {0.0, 1.0, 0.0}};
    pdef.slowdown = 1.0f;
    IColor color(155, 244, 228);
    pdef.color = {{FColor(color).rgb(), FColor(0.2f, 0.2f, 0.8f).rgb()}};
    pdef.textureName = "circular.png";

    SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef), 0.0f, 0.5f);

    ssdef.prepareFunc = [](AnimationContext &ctx, EmissionState &em) {
      auto ret = defaultPrepareEmission(ctx, em);
      auto &parts = ctx.ps.subSystems[0].particles;
      if (parts.empty())
        return 0.0f;
      em.strengthMin = em.strengthMax = em.strengthMax * (1.2f - parts.front().particleTime());
      return ret;
    };

    ssdef.emitFunc = [](AnimationContext &ctx, EmissionState &em, Particle &pinst) {
      defaultEmitParticle(ctx, em, pinst);
      auto &parts = ctx.ps.subSystems[0].particles;
      if (!parts.empty()) {
        auto &gpart = parts.front();
        pinst.pos += gpart.pos + gpart.particleTime() * ctx.ps.targetOff;
      }
    };
    psdef.subSystems.emplace_back(ssdef);
  }

  psdef.name = "magic_missile";
  mgr.addDef(psdef);
}

static void addSleepEffect(FXManager &mgr) {
  // TODO: tutaj trzeba zrobić tak, żeby cząsteczki które spawnują się później
  // zaczynały z innym kolorem
  EmitterDef edef;
  edef.strengthMin = edef.strengthMax = 20.0f;
  edef.angle = -fconstant::pi * 0.5f;
  edef.angleSpread = 0.2f;
  edef.frequency = 3.0f;
  edef.source = FRect(-2, -8, 2, -5);

  ParticleDef pdef;
  pdef.life = 2.0f;
  pdef.size = 10.0f;
  pdef.alpha = {{0.0f, 0.5f, 1.0f}, {0.0, 1.0, 0.0}, InterpType::cosine};

  pdef.color = FVec3(1.0f);
  pdef.textureName = "special_4x1.png";
  pdef.textureTiles = {4, 1};

  SubSystemDef ssdef(mgr.addDef(pdef), mgr.addDef(edef), 0.0f, 1.0f);
  ssdef.emitFunc = [](AnimationContext &ctx, EmissionState &em, Particle &pinst) {
    defaultEmitParticle(ctx, em, pinst);
    pinst.texTile = {0, 0};
    pinst.rot = ctx.rand.getDouble(-0.2f, 0.2f);
  };

  ParticleSystemDef psdef;
  psdef.subSystems = {ssdef};
  psdef.name = "sleep";
  psdef.isLooped = true;
  psdef.animLength = 1.0f;
  mgr.addDef(psdef);
}

void FXManager::addDefaultDefs() {
  addTestSimpleEffect(*this);
  addTestMultiEffect(*this);
  addWoodSplinters(*this);
  addRockSplinters(*this);
  addRockCloud(*this);
  addExplosionEffect(*this);
  addRippleEffect(*this);
  addCircularBlast(*this);
  addFeetDustEffect(*this);
  addMagicMissleEffect(*this);
  addSleepEffect(*this);
};
}
